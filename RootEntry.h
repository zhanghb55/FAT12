#pragma once

#include "FAT.h"
#pragma pack (1) /*指定按1字节对齐*/
struct RootEntry{
    char DIR_Name[11];//文件名与扩展名
    char DIR_Attr;//文件属性
    char reserve[10];//保留位
    short DIR_WrtTime;//最后一次写入时间
    short DIR_WrtDate;//最后一次写入日期
    short DIR_FstClus;//文件开始的簇号
    int DIR_FileSize;//文件大小
};
void convert_input(char*str){
    for(int i = 0;i < 11;i++){
        if(str[i] >= 'a' && str[i] <= 'z'){
            str[i] -= ('a' - 'A');
        }
    }
}
//获取RootEntry类型中的文件名
bool convert_name(struct RootEntry*root,char*realname){
    int name_len = 0;
    int flag = 0;
    for(int i = 0;i < 11;i++){
        if(root->DIR_Name[i] == ' ' || root->DIR_Name[i] == '.' || (root->DIR_Name[i] >= 'A' && root->DIR_Name[i] <= 'Z') || (root->DIR_Name[i] >= '0' && root->DIR_Name[i] <= '9')|| (root->DIR_Name[i] >= 'a' && root->DIR_Name[i] <= 'z')){
            if(root->DIR_Name[i] == ' '){
                while(root->DIR_Name[i] == ' '){
                    i++;
                }
                i--;
                if(root->DIR_Attr != 0x10){
                    realname[name_len] = '.';
                    name_len++;
                }
            }
            else{
                realname[name_len++] = root->DIR_Name[i];
            }
        }
        else{
            flag = 1;
            break;
        }
    }
    realname[name_len] = '\0';
    return flag;   
}

//用于匹配输入的文件名与目录表中是否有相同的
struct RootEntry* match_by_name(unsigned char*Entry_Table,char*target_name,int count_of_entry_table,bool*match){
    struct RootEntry*temp = (struct RootEntry*)malloc(sizeof(struct RootEntry));
    int base = 0;
    for(int i = 0;i < count_of_entry_table;i++){
        memcpy(temp,(const void*)(Entry_Table + base),32);//把目录项拷进来
        base += Size_Of_A_Root_Entry;
        char realname[12];
        memset(realname,0,12);
        bool ret = convert_name(temp,realname);

        if(ret){
            continue;
        }
        if(strcmp(target_name,realname) == 0){
            *match = true;
            return temp;
        }
    }
    return NULL;
}

//用于匹配输入的文件名与目录表中是否有相同的,这个函数为了删除而设计，多一个参数用于返回目录的位置
struct RootEntry* match_by_name_for_del(unsigned char*Entry_Table,char*target_name,int count_of_entry_table,bool*match,int*count){
    struct RootEntry*temp = (struct RootEntry*)malloc(sizeof(struct RootEntry));
    int base = 0;
    for(int i = 0;i < count_of_entry_table;i++){
        memcpy(temp,(const void*)(Entry_Table + base),32);//把目录项拷进来
        base += Size_Of_A_Root_Entry;
        char realname[12];
        memset(realname,0,12);
        bool ret = convert_name(temp,realname);
        if(ret){
            continue;
        }
 
        if(strcmp(target_name,realname) == 0){
            *match = true;
            *count = i;
            return temp;
        }
    }
    return NULL;
}

//获取RootEntry类型中的修改时间
void convert_time(struct RootEntry*root,char*dest){
    int year = (unsigned short)root->DIR_WrtDate >> 9;
    year += 1980;
    int month = (unsigned short)root->DIR_WrtDate >> 5;
    month = month & 15;
    int day = (unsigned short)root ->DIR_WrtDate & 31;
    int hour = (unsigned short)root -> DIR_WrtTime >> 11;
    int min = (unsigned short)root ->DIR_WrtTime >> 5;
    min = min & 63;
    sprintf(dest,"%d-%d-%d %d:%d",year,month,day,hour,min);
}
//打印某个目录项
void print_a_entry(struct RootEntry*root){
    char realname[12];
    int name_len = 0;
    int flag = convert_name(root,realname);
    if(flag == 0){
        printf("%-12s",realname);
        char*time = (char*)malloc(20);
        memset(time,0,20);
        convert_time(root,time);
        if(root->DIR_Attr == 0x10){
            printf("<dir> ");
        }
        else{
            printf("      ");
        }
        printf("    %14s    ",time);
        if(root->DIR_Attr != 0x10)
        printf("%dB\n",root->DIR_FileSize);
        else printf("\n");
        free(time);
    }
}

//打印根目录表或子目录表
void print_entry_table(unsigned char*File,int count_of_entry){
    //偏移量初始化为0，因为传递的参数是从根目录所在的第一个扇区开始的
    int base = 0;
    for(int x = 0;x < count_of_entry;x++){
        //每次申请一个root空间
        struct RootEntry*root = (struct RootEntry*)malloc(32);
        memcpy(root,(const void*)(File + base),32);//把根目录项拷进来
        base += 32;//偏移量加32，使下次读取是下一个位置
        if(root->DIR_Name[0] == 0)break;//如果目录项第一个字节是0则说明根目录结束了
        print_a_entry(root);
        free(root);
    }    
}

void print_table(FILE*boot,int cur_pos,int cur_size){
    unsigned char*File = (unsigned char*)malloc(sizeof(char)*cur_size*Sector_Size);
    //FILE*boot;
    //boot = fopen ("dossys.img", "rb");
    fseek(boot,cur_pos*Sector_Size,SEEK_CUR);
    fread((void*)File,1,cur_size*Sector_Size,boot);
    print_entry_table(File,224);
    free(File);
}

unsigned char*get_root(char*img_name){
    FILE*boot = fopen(img_name,"rb"); 
    unsigned char*File = (unsigned char*)malloc(sizeof(char)*14*Sector_Size);
    fseek(boot,19*Sector_Size,SEEK_CUR);
    fread((void*)File,1,14*Sector_Size,boot);
    fclose(boot);
    return File;
}
void show_root(char*img_name,int cur_pos,int cur_size){
    FILE*boot = fopen(img_name,"rb"); 
    print_table(boot,cur_pos,cur_size);
    fclose(boot);    
}

unsigned char*get_normal_entry(char*img_name,int begin,int*size){
    unsigned short* FAT = get_Fat_convert(img_name);
    struct file_cluster Entry_FAT =  Build_File(FAT,begin);
    *size = Entry_FAT.count;
    unsigned char* Entry_Sectors = (unsigned char*)malloc(Entry_FAT.count*Sector_Size);
    for(int i = 0;i < Entry_FAT.count;i++){
        FILE*Sector = fopen(img_name,"rb+");
        fseek(Sector,(Entry_FAT.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fread(Entry_Sectors + i * Sector_Size,1,Sector_Size,Sector);
        fclose(Sector);
    }
    return Entry_Sectors;
}
void show_normal_entry(char*img_name,int begin){
    int size = 0;
    unsigned char*Entry = get_normal_entry(img_name,begin,&size);
    print_entry_table(Entry,size * Sector_Size / Size_Of_A_Root_Entry);
    free(Entry);
}

//返回是否在目录中找到空目录项并把空目录项以order指针返回
bool find_new_empty_pos_in_entry(unsigned char*entry,int size,int*order){
    int num_of_entry = size * Sector_Size / Size_Of_A_Root_Entry;
    for(int i = 0;i < num_of_entry;i++){
        if(entry[i * Size_Of_A_Root_Entry] == 0 || entry[i * Size_Of_A_Root_Entry] == 229){
            *order = i;
            return true;
        }
    }
    return false;
}
bool match_name(char name1[8],char name2[8]){//返回true说明二者相同
    bool flag = true;
    for(int i = 0;i < 8;i++){
       // if(name1[i] == 0|| name2[i] == 0)break;
        if(name1[i] != name2[i]){
            flag = false;
        }
    }
    return flag;
}
bool new_file_name_match(unsigned char*cur_entry,int size,char*file_name){
    char name_8_bit[8];
    for(int i = 0;i < 8;i++)name_8_bit[i] = 0x20;
    bool point = false;
    int point_pos = 0;
    for(int i = 0;i < 11;i++){
        if(file_name[i] == '.'){
            point_pos = i;
            point = true;
        }
    }
    if(point){
        for(int i = 0;i < point_pos;i++){
            name_8_bit[i] = file_name[i];
        }
        char exist[8];
        for(int i = 0;i < size * Sector_Size / Size_Of_A_Root_Entry;i++){
            memcpy(exist,cur_entry + i * Size_Of_A_Root_Entry,8);
            if(match_name(name_8_bit,exist)){
                return true;
            }
        }
    }
    else
    {
        for(int i = 0;i < strlen(file_name);i++){
            name_8_bit[i] = file_name[i];
        }
        char exist[8];
        for(int i = 0;i < size * Sector_Size / Size_Of_A_Root_Entry;i++){
            memcpy(exist,cur_entry + i * Size_Of_A_Root_Entry,8);
 /*           for(int i = 0;i < 8;i++){
                printf("%c",exist[i]);
            }
            printf("\t");
            for(int i = 0;i < 8;i++){
                printf("%c",name_8_bit[i]);
            }
            printf("\n");*/
            if(match_name(name_8_bit,exist)){
                return true;
            }
        }        
    }
    return false;
}
