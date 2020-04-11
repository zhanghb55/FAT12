#pragma once
#include "Macro.h"
#include "FAT.h"
#include "RootEntry.h"
#include "MBR.h"



char* Read_File(struct RootEntry*entry,char*img_name){
    unsigned short* FAT_Table = get_Fat_convert(img_name);


    unsigned short begin = entry->DIR_FstClus;

    struct file_cluster res = Build_File(FAT_Table,begin);//将整个文件根据FAT表构造出来

    
    unsigned int len = entry->DIR_FileSize;//文件的长度，可以考虑用

    char* file = (char*)malloc(len + 1);
    memset(file,0,len + 1);
    FILE* Cluster = fopen(img_name,"rb");
    

    for(int i = 0;i < res.count - 1;i++){
        fseek(Cluster,(31 + res.clusters[i]) * Sector_Size,SEEK_CUR);
        fread(file + i * Sector_Size,1,Sector_Size,Cluster);
    }
    fseek(Cluster,(31 + res.clusters[res.count - 1]) * Sector_Size,SEEK_CUR);
    fread(file + (res.count - 1)* Sector_Size,1,len % Sector_Size,Cluster);

    fclose(Cluster);
    free(FAT_Table);
    return file;
}
