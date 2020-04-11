#pragma once

#include "gedit.h"

bool mkdir(unsigned char*cur_entry,int cur_pos,int cur_size,char*img_name,unsigned short*FAT){
    int begin = cur_pos;
    char input[13];
    scanf("%s",input);
    convert_input(input);
    if(new_file_name_match(cur_entry,cur_size,input))return -1;

    int pos = 0;
    struct RootEntry new_entry;
    bool new_pos = find_new_empty_pos_in_entry(cur_entry,cur_size,&pos);

    struct file_cluster entry_cluster = Build_File(FAT,cur_pos - 31);

    bool name_is_correct = convert_input_string_to_entry_name(input,new_entry.DIR_Name);
    if(!name_is_correct){
   
        return 0;//名字不符合规则
    }   
    if(cur_pos != 19 && pos >= 16){
        cur_pos = entry_cluster.clusters[pos / 16] + 31;
        pos = pos % 16;
    }
    if(new_pos == false && cur_pos != 19){
        int new_pos;
        add_a_sector(cur_pos - 31,FAT,&new_pos);
        cur_size++;
        free(cur_entry); 
        struct RootEntry temp;
        temp.DIR_FstClus = cur_pos;
        int begin_pos = temp.DIR_FstClus;
        cur_entry = Read_Entry(begin_pos,img_name,&cur_pos,&cur_size);
        cur_pos = new_pos + 31;
    }
    short day,time;
    get_time_and_convert(&day,&time);

    unsigned char*msg = (unsigned char*)malloc(2 * Sector_Size);

    struct RootEntry basic_entry1,basic_entry2;
    for(int i = 0;i < 11;i++){
        basic_entry1.DIR_Name[i] = 0x20;
        basic_entry2.DIR_Name[i] = 0x20;
    }
    basic_entry1.DIR_Name[0] = '.';
    basic_entry2.DIR_Name[0] = '.';
    basic_entry2.DIR_Name[1] = '.';

    basic_entry1.DIR_Attr = 0x10;
    basic_entry2.DIR_Attr = 0x10;

    basic_entry1.DIR_FileSize = 0;
    basic_entry2.DIR_FileSize = 0;

    basic_entry1.DIR_WrtDate = day;
    basic_entry2.DIR_WrtDate = day;

    basic_entry1.DIR_WrtTime = time;
    basic_entry2.DIR_WrtTime = time;
    

    struct file_cluster res;
    find_FAT_pos(FAT,2 * Size_Of_A_Root_Entry,&res);
    basic_entry1.DIR_FstClus = res.clusters[0];
    if(begin != 19)basic_entry2.DIR_FstClus = begin - 31;
    else basic_entry2.DIR_FstClus = 0;

    memcpy(msg,&basic_entry1,32);
    memcpy(msg + Size_Of_A_Root_Entry,&basic_entry2,32);

    write_a_new_file_to_FAT(FAT,res,img_name);
    clean(res,img_name);
    wirte_entry_to_sector(res,msg,2 * Size_Of_A_Root_Entry,img_name);

    new_entry.DIR_FstClus = res.clusters[0];
    new_entry.DIR_Attr = 0x10;
    new_entry.DIR_FileSize = 0;
    new_entry.DIR_WrtDate = day;
    new_entry.DIR_WrtTime = time;    

    memcpy(cur_entry + pos * Size_Of_A_Root_Entry,(void*)&new_entry,Size_Of_A_Root_Entry);


    //将目录项写回文件
    FILE* wirte_entry = fopen(img_name,"rb+");
    fseek(wirte_entry,cur_pos * Sector_Size + pos * Size_Of_A_Root_Entry,SEEK_CUR);
    fwrite(&new_entry,1,Size_Of_A_Root_Entry,wirte_entry);

    fclose(wirte_entry);
    free(msg);
    return 1;//成功计算并返回
}