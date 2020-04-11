#include "FAT.h"

bool test_single_file(unsigned short*FAT,int begin){
    if(begin == 0xfff || begin == 0xff0)return true;
    int cur = begin;
    int count = 0;
    while(FAT[cur] != 0xFFF){
        if(FAT[cur] == 0xFFF || FAT[cur] == 0xFF0)return true;
        if(count >= 3072 || FAT[cur] == 0){
            return false;
        }
        cur = FAT[cur];
    }
}

bool test_all_FAT(unsigned short*FAT){
    for(int i = 2;i < 3072;i++){
        if(FAT[i] != 0){
            if(test_single_file(FAT,FAT[i]) == false){
                return false;
            }
        }
    }
    return true;
}

bool test_FAT(char*img_name){
    unsigned short*Fat_after_convert = (unsigned short*)malloc(sizeof(unsigned short)*Num_Of_Fat);
    memset(Fat_after_convert,0,sizeof(unsigned short)*Num_Of_Fat);//初始化结果
    FILE*boot = fopen(img_name,"rb"); 
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
    free(Fat_after_convert);
    fclose(boot);
    free(Fat);
    if(flag == 0)return false;

    unsigned short* FAT = get_Fat_convert(img_name);
    bool res = test_all_FAT(FAT);
    free(FAT);
    return res;
}