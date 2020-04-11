#pragma once

#include "Find_entry.h"


void tree(int cur_pos,int cur_size,int count,char*img_name,unsigned short*FAT){
    unsigned char* cur_entry = Read_Entry(cur_pos - 31,img_name,&cur_pos,&cur_size);
    struct RootEntry all_entry[cur_size * Sector_Size / Size_Of_A_Root_Entry];
    memcpy(all_entry,cur_entry,cur_size * Sector_Size);
    free(cur_entry);
    for(int i = 0;i < cur_size * Sector_Size / Size_Of_A_Root_Entry;i++){
        unsigned int x = (unsigned char)all_entry[i].DIR_Name[0];
        if(x == 0)return;
        else if(x == 229){
            continue;
        }
        else if(all_entry[i].DIR_Attr != 0x10){
            char real_name[12];
            convert_name(&all_entry[i],real_name);
            for(int i = 0;i < count;i++){
                printf("   ");
            }
            printf("|%s\n",real_name);
            continue;
        }
        else if(all_entry[i].DIR_Attr == 0x10){
            char real_name[12];
            convert_name(&all_entry[i],real_name);
            for(int i = 0;i < count;i++){
                printf("   ");
            }
            printf("|%s\n",real_name);
            int begin = all_entry[i].DIR_FstClus;
            struct file_cluster next_entry = Build_File(FAT,begin);

            if(all_entry[i].DIR_Name[0] != '.')
            tree(begin + 31,next_entry.count,count + 1,img_name,FAT);
        }
    }
    return;
}
