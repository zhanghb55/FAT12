#pragma once
#include "FAT.h"
#include "MBR.h"
#include "RootEntry.h"
#include "Macro.h"
#include "Find_entry.h"
#include "Find_file.h"

bool del_file(unsigned char*entry_table,char*target,int cur_size,unsigned short*FAT,int*pos){
    bool match = false;
    struct RootEntry* entry = match_by_name_for_del(entry_table,target,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match, pos);
    if(match){
        if(entry->DIR_Attr != 0x10){
            entry->DIR_Name[0] = 0xE5;
            int begin = entry->DIR_FstClus;
            int temp = begin;
            while (FAT[temp] != 0xFFF)
            {
                if(FAT[temp] == 0)break;
                int hold = temp;
                temp = FAT[temp];
                FAT[hold] = 0;
            }
            FAT[temp] = 0;
            return true;
        }
    }
    return false;
}
