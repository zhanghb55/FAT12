#pragma

#include "delete.h"
//unsigned char* Read_Entry(int begin,char*img_name,int*cur_pos_ptr,int*cur_size_ptr)
int count_of_entry(int begin,char*img_name,int size){
    int count = 0;
    int temp_pos = 0;
    unsigned char* entry = Read_Entry(begin,img_name,&temp_pos,&size);
    for(int i = 0;i < size * Sector_Size / Size_Of_A_Root_Entry;i++){
        if(entry[i * Size_Of_A_Root_Entry] != 0 && entry[i * Size_Of_A_Root_Entry] != 0xE5)count++;
    }

    free(entry);
    return count;
}
//return -1找不到该目录，return 0目录非空
int rd(unsigned char*cur_entry,int cur_pos,int cur_size,char*img_name,unsigned short*FAT,char*target){
    convert_input(target);
    int pos;
    bool res = false;
    match_by_name_for_del(cur_entry,target,cur_size * Sector_Size / Size_Of_A_Root_Entry,&res,&pos);
    if(res == false){
        return -1;//找不到该目录
    }
    struct RootEntry target_entry;
    memcpy(&target_entry,cur_entry + pos * Size_Of_A_Root_Entry,Size_Of_A_Root_Entry);
    if(target_entry.DIR_Attr != 0x10)return -1;
    int count = count_of_entry(target_entry.DIR_FstClus,img_name,cur_size);
    if(count > 2)return 0;


    target_entry.DIR_Name[0] = 0xE5;
    int begin = target_entry.DIR_FstClus;
    int temp = begin;

    while (FAT[temp] != 0xFFF)
    {
        if(FAT[temp] == 0)break;
        int hold = temp;
        temp = FAT[temp];
        FAT[hold] = 0;
    }
    FAT[temp] = 0;
    write_back(img_name,FAT);
    fopen(img_name,"rb+");

    FILE * E5 = fopen(img_name,"rb+");
    if(cur_pos == 19){
        fseek(E5,cur_pos * Sector_Size + pos * Size_Of_A_Root_Entry,SEEK_CUR);
        unsigned char a[0];
        a[0] = 0xE5;
        fwrite(a,1,1,E5);
        fclose(E5);
    }
    else{
        struct file_cluster del_entry = Build_File(FAT,cur_pos - 31);
        int cur_pos1 = del_entry.clusters[pos / (Sector_Size / Size_Of_A_Root_Entry)];
        cur_pos1 += 31;
        pos = pos % (Sector_Size / Size_Of_A_Root_Entry);
        fseek(E5,cur_pos1 * Sector_Size + pos * Size_Of_A_Root_Entry,SEEK_CUR);
        unsigned char a[0];
        a[0] = 0xE5;
        fwrite(a,1,1,E5);
        fclose(E5);
    }
    return 1;
}