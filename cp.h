#pragma once

#include "path.h"
#include "Find_file.h"
#include "gedit.h"
/*
//传递源簇和目标簇
void copy_sector(char*img_name,int dest,int source,unsigned short*FAT){
    struct file_cluster source_file = Build_File(FAT,source);
    struct file_cluster dest_file = Build_File(FAT,dest);
    char*info = (char*)malloc(source_file.count * Sector_Size);
    memset(info,0,source_file.count * Sector_Size);
    FILE*read = fopen(img_name,"rb");
    for(int i = 0;i < source_file.count;i++){
        fseek(read,(source_file.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fread(info + i * Sector_Size,1,Sector_Size,read);
    }

    fclose(read);
    FILE*write = fopen(img_name,"rb+");
    for(int i = 0;i < dest_file.count;i++){
        fseek(write,(dest_file.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fwrite(info + i * Sector_Size,1,Sector_Size,write);
    }    

    fclose(write);
    free(info);
}
*/


//return -1目标目录错误,return -2表示源文件目录错误,return 0表示无法拷贝，返回1表示拷贝成功
int cp(char*dest,char*source,char*img_name,unsigned short*FAT){
    int dest_res_pos,dest_res_size;
    struct entry_detial list;
    struct RootEntry dest_res;
    int dest_ret = analyse_path(dest,&dest_res_pos,&dest_res_size,&list,img_name,&dest_res,true);
    if(dest_ret != 1)return -1;
    int source_res_pos,source_res_size;
    struct RootEntry source_res;
    int source_ret = analyse_path(source,&source_res_pos,&source_res_size,&list,img_name,&source_res,true);    
    if(source_ret != 2)return -1;

    int len = source_res.DIR_FileSize;
    char *file = Read_File(&source_res,img_name);
    unsigned char*cur_entry = Read_Entry(dest_res_pos - 31,img_name,&dest_res_pos,&dest_res_size);
    //int creat_new_file_entry(char*input,unsigned char*cur_entry,int cur_pos,int cur_size,char*img_name,unsigned short*FAT,struct entry_detial*list){

    int final_ret = creat_new_file_entry_for_cp(source,cur_entry,dest_res_pos,dest_res_size,img_name,FAT,file,len);
    
    if(final_ret != 1)return 0;
    else return 1;
}