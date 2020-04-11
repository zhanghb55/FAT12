#pragma once
#include "Macro.h"

#pragma pack(1)
struct MBR
{
    char BS_OEMName[8];    // OEM字符串，必须为8个字符，不足以空格填空
    short BPB_BytsPerSec; // 每扇区字节数
    char BPB_SecPerClus;  // 每簇占用的扇区数
    short BPB_RsvdSecCnt; // Boot占用的扇区数
    char BPB_NumFATs;     // FAT表的记录数
    short BPB_RootEntCnt; // 最大根目录文件数
    short BPB_TotSec16;   // 扇区总数
    char BPB_Media;       // 媒体描述符
    short BPB_FATSz16;    // 每个FAT占用扇区数
    short BPB_SecPerTrk;  // 每个磁道扇区数
    short BPB_NumHeads;   // 磁头数
    int BPB_HiddSec;      // 隐藏扇区数
    int BPB_TotSec32;     // 如果BPB_TotSec16是0，则在这里记录
    char BS_DrvNum;       // 中断13的驱动器号
    char BS_Reserved1;    // 未使用
    char BS_BootSig;      // 扩展引导标志
    int BS_VolID;         // 卷序列号
    char BS_VolLab[11];    // 卷标，必须是11个字符，不足以空格填充
    char BS_FileSysType[8];// 文件系统类型，必须是8个字符，不足填充空格
};
#pragma pack()
void print_MBR(struct MBR*temp){
    printf("Disk Name:%s\n",temp->BS_OEMName);
    printf("Sector Size:%d\n",temp->BPB_BytsPerSec);
    printf("Sectors per cluster:%d\n",temp->BPB_SecPerClus);
    printf("Number of sectors occupied by boot:%d\n",temp->BPB_RsvdSecCnt);
    printf("Number of FATs:%d\n",temp->BPB_NumFATs);
    printf("The max capacity of root entry:%d\n",temp->BPB_RootEntCnt);
    
    printf("The number of all Sectors:%d\n",temp->BPB_TotSec16);
    //printf("Media Descriptor:%c\n",temp->BPB_Media);
    printf("The number of Sectors of each Fat table:%d\n",temp->BPB_FATSz16);
    printf("Number of sectors per track:%d\n",temp->BPB_SecPerTrk);
    printf("The number of magnetic read head:%d\n",temp->BPB_NumHeads);
    printf("Number of hidden sectors:%d\n",temp->BPB_HiddSec);
    printf("Volume serial number:%d\n",temp->BS_VolID);
    printf("Volume label:");
    for(int i = 0;i < 11;i++){
        printf("%c",temp->BS_VolLab[i]);
    }
    printf("\n");

    printf("File system type:");
    for(int i = 0;i < 8;i++){
        printf("%c",temp->BS_FileSysType[i]);
    }
    printf("\n");
}
struct MBR*get_MBR_info(FILE*File){
    struct MBR* temp = malloc(Sector_Size);
    fseek(File,3,SEEK_CUR);
    fread(temp,1,Sector_Size,File);
    return temp;
}

void show_MBR(char*image_name){
    FILE*boot = fopen(image_name,"rb");
    struct MBR* temp = get_MBR_info(boot);
    print_MBR(temp);
    free(temp);
    fclose(boot);
}
