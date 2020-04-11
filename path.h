#pragma once


#include "FAT.h"
#include "MBR.h"
#include "RootEntry.h"
#include "Macro.h"
#include "Find_entry.h"
#include "Find_file.h"

struct entry_detial
{
    int size;
    char name[20][20];
};
//return 0 表示非路径，return -1表示无效路径， return 1表示找寻成功，目标为目录，return 2说明寻找成功，目标为文件
int analyse_path(char*target,int *res_pos,int *res_size,struct entry_detial*list0,char*img_name,struct RootEntry*final_res,int ret_or_not){
    struct entry_detial*list = (struct entry_detial*)malloc(sizeof(struct entry_detial));
    list->size = 0;
    if(target[0] != '\\')return 0;
    int cur_pos = 19;
    int cur_size = 14;
    int count = 0;
    short pos_of_line[100];

    FILE* entry_file = fopen(img_name,"rb");
    fseek(entry_file,cur_pos * Sector_Size,SEEK_CUR);    
    unsigned char* cur_entry = (unsigned char*)malloc(14 * Sector_Size);//读取根目录扇区存储到char数组
    fread(cur_entry,1,14 * Sector_Size, entry_file);
    fclose(entry_file);

    memset(pos_of_line,0,sizeof(short) * 100);
    for(int i = 0;i < strlen(target);i++){
        if(target[i] == '\\'){
            pos_of_line[count++] = i;
        }
    }
    for(int i = 0;i < count - 1;i++){
        char next[13];
        memset(next,0,13);
        memcpy(next,target + pos_of_line[i] + 1,pos_of_line[i + 1] - pos_of_line[i] - 1);

        convert_input(next);
        bool match = false;
        struct RootEntry*next_entry = match_by_name(cur_entry,next,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match);
        //printf("%d   %d\n",cur_pos,cur_size);
        if(match == false){
            free(cur_entry); 
            free(list);   
            return -1;
        }
        free(cur_entry);
        cur_entry = Read_Entry(next_entry->DIR_FstClus,img_name,&cur_pos,&cur_size);
        strcpy((*list).name[(*list).size],next);
        (*list).size++;
    }
    char next[13];
    memset(next,0,13);
    memcpy(next,target + pos_of_line[count - 1] + 1,strlen(target) - pos_of_line[count - 1]);    
    convert_input(next);

        
    bool match = false;
    struct RootEntry*next_entry = match_by_name(cur_entry,next,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match);
    if(match == false){ 
        memset(target,0,strlen(target));
        strcpy(target,next);
        free(cur_entry);
        free(list);
        //用ret_or_not确定是否将倒数第二层目录确定为当前目录
        if(ret_or_not){
            *res_pos = cur_pos;
            *res_size = cur_size;
        }
        return -2;
    }

    memcpy(final_res,next_entry,Size_Of_A_Root_Entry);
    if(next_entry->DIR_Attr != 0x10){
        memset(target,0,strlen(target));
        strcpy(target,next);
        *res_pos = cur_pos;
        *res_size = cur_size;
        free(cur_entry);
        free(list);
        return 2;
    }
    else{
        cur_entry = Read_Entry(next_entry->DIR_FstClus,img_name,&cur_pos,&cur_size);
        strcpy((*list).name[(*list).size],next);
        (*list).size++;        
        *res_pos = cur_pos;
        *res_size = cur_size;
        free(cur_entry);
        list0->size = list->size;
        for(int i = 0;i < list->size;i++){
            strcpy(list0->name[i],list->name[i]);
        }
        free(list);
        return 1;        
    }
}
