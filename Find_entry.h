#pragma once
#include "Macro.h"
#include "RootEntry.h"
#include "FAT.h"




unsigned char* Read_Entry(int begin,char*img_name,int*cur_pos_ptr,int*cur_size_ptr){
    unsigned short* FAT_Table = get_Fat_convert(img_name);
    if(begin != 0){
        *cur_pos_ptr = begin + 31;
        unsigned char* res = get_normal_entry(img_name,begin,cur_size_ptr);
        return res;
    }
    else{

        *cur_pos_ptr = Sector_Of_MBR + Num_Of_Fat_Table * Sector_Of_Fat_Table;
        *cur_size_ptr = Sector_Of_Root_Entry;
        return NULL;  
    }
}
