#pragma once
#include "Macro.h"

struct file_cluster{
    int count;
    short clusters[Num_Of_Fat];
};

struct file_cluster Build_File(unsigned short* FAT_Table,int begin_postion){
    struct file_cluster res;
    res.count = 0;
    res.clusters[res.count++] = begin_postion;
    while(FAT_Table[res.clusters[res.count - 1]] != 0xfff && FAT_Table[res.clusters[res.count - 1]] != 0xff0){
        res.clusters[res.count] = FAT_Table[res.clusters[res.count - 1]];
        res.count++;
    }
    return res;
}


//两个参数分别是原来的FAT表
//以及转换后的簇连接关系
void convert_Fat(unsigned char*Fat,unsigned short*FAT_After_Convert){
    for(int i = 0;i < Num_Of_Fat;i++){
        //奇数的FAT项，在后一个字节的整个字节的后面接上前一个字节的高字节
        //偶数的FAT项，在前一个字节的前面接上后一个字节的低位
        //例如03 40 00，可以解读为003，004
        if(i % 2){
            int index = i * 1.5;
            unsigned short temp = (unsigned char)Fat[index + 1];
            temp = temp << 4;
            FAT_After_Convert[i] += temp;
            temp = (unsigned char)Fat[index];
            temp = temp >> 4;
            FAT_After_Convert[i] += temp;
        }
        else{
            int index = i * 1.5;
            FAT_After_Convert[i] += (unsigned char)Fat[index];
            unsigned short temp = (unsigned char)Fat[index + 1];
            temp = temp << 8;
            temp = temp & 4095;
            FAT_After_Convert[i] += temp;
        }
    }
}

unsigned short*get_Fat_convert(char*imgname){
    unsigned short*Fat_after_convert = (unsigned short*)malloc(sizeof(unsigned short)*Num_Of_Fat);
    memset(Fat_after_convert,0,sizeof(unsigned short)*Num_Of_Fat);//初始化结果
    FILE*boot = fopen(imgname,"rb"); 
    unsigned char*Fat = (unsigned char*)malloc(2*Sector_Of_Fat_Table * Sector_Size);
    fseek(boot,Sector_Size*Sector_Of_MBR,SEEK_CUR);
    fread(Fat,1,2*Sector_Of_Fat_Table * Sector_Size,boot);

    int flag = 1;
    //对比两个FAT表，如果有不同说明FAT出错
    for(int i = 0;i < Sector_Of_Fat_Table * Sector_Size;i++){
        if(Fat[i] != Fat[i + Sector_Of_Fat_Table * Sector_Size]){
            flag = 0;
            break;
        }
    }
    if(flag){
        //无误则有对应关系
        convert_Fat(Fat,Fat_after_convert);
    }    
    else{
        printf("Something wrong in FAT.\n");
    }
    free(Fat);
    fclose(boot);
    return Fat_after_convert;
}

void convert_Fat_to_write_back(unsigned short*Converted_FAT,unsigned char* target){
    for(int i = 0;i < Num_Of_Fat;i++){
        //奇数的FAT项，在后一个字节的整个字节的后面接上前一个字节的高字节
        //偶数的FAT项，在前一个字节的前面接上后一个字节的低位
        //例如03 40 00，可以解读为003，004
        int val = Converted_FAT[i];
        if(i % 2){
            unsigned char low4 = Converted_FAT[i] & 15;
            low4 = low4 << 4;
            unsigned char high8 = Converted_FAT[i] >> 4; 
			int temp = i * 1.5;  
            target[temp] = target[temp] & 0x0F;
            target[temp] += low4;
            target[temp + 1] = high8;
        }
        else{
            unsigned char low8 = Converted_FAT[i] & 255;
            unsigned char high4 = Converted_FAT[i] >> 8;
            int temp = i * 1.5; 
            target[temp] = low8;
            target[temp + 1] = target[temp + 1] & 0xF0;
            target[temp + 1] += high4;
        }
    }    
}
void write_back(char*imgname,unsigned short*Converted_FAT){
    FILE*boot1 = fopen(imgname,"rb+"); 
    FILE*boot2 = fopen(imgname,"rb+"); 
    unsigned char* New_Fat = (unsigned char*)malloc(Sector_Of_Fat_Table * Sector_Size);
    memset(New_Fat,0,Sector_Of_Fat_Table * Sector_Size);
    convert_Fat_to_write_back(Converted_FAT,New_Fat);

    fseek(boot1,Sector_Size*Sector_Of_MBR,SEEK_CUR);
    fwrite(New_Fat,1,Sector_Of_Fat_Table * Sector_Size,boot1);
    fseek(boot2,Sector_Size*(Sector_Of_MBR + Sector_Of_Fat_Table),SEEK_CUR);
    fwrite(New_Fat,1,Sector_Of_Fat_Table * Sector_Size,boot2);
    free(New_Fat);
    fclose(boot1);
    fclose(boot2);
}

bool add_a_sector(int begin_pos,unsigned short*FAT,int*new_pos){
    if(begin_pos == 19)return false;
    struct file_cluster entry_file = Build_File(FAT,begin_pos);
    short target;
    bool match = false;
    for(int i = 2;i < Num_Of_Fat;i++){
        if(FAT[i] == 0){
            target = i;
            match = true;
            break;
        }
    }
    *new_pos = target;

    if(match == false)return false;
    short last_pos = entry_file.clusters[entry_file.count - 1];

    FAT[last_pos] = target;
    FAT[target] = 0xFFF;

    return true;
}

