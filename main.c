#include "FAT.h"
#include "MBR.h"
#include "RootEntry.h"
#include "Macro.h"
#include "Find_entry.h"
#include "Find_file.h"
#include "delete.h"
#include "gedit.h"
#include "mkdir.h"
#include "rd.h"
#include "tree.h"
#include "path.h"
#include "cp.h"
#include "test_FAT.h"
//#define Name "boot.img"





int main(){
	char Name[20];
	printf("Please input img name:"); 
	scanf("%s",Name);
    printf("SYSU-FAT12  Version 3.0\n");
    printf("            (C)Copyright ZHB 2020-04\n");
	
    int cur_pos = 19;//确定当前目录的起始扇区
    int cur_size = 14;//确定当前目录的大小
    FILE* entry_file = fopen(Name,"rb");
    fseek(entry_file,cur_pos * Sector_Size,SEEK_CUR);    
    unsigned char* root = (unsigned char*)malloc(14 * Sector_Size);//读取根目录扇区存储到char数组
    fread(root,1,14 * Sector_Size, entry_file);
    fclose(entry_file);
    unsigned char * cur_entry = root;
    struct entry_detial list;
    list.size = 0;
    while (1)
    {
        printf("A:>");
        for(int i = 0;i < list.size;i++){
            if(i > 0)printf("/");
            printf("%s",list.name[i]);
        }
        printf(" ");
        char command[15];
        scanf("%s",command);
        if(!strcmp(command,"FAT")){
            bool test = test_FAT(Name);
            if(test){
                printf("There is nothing wrong with FAT.\n");
            }
            else{
                printf("There is something wrong with FAT, do you want to quit the program?Y/N\n");
                char s;
                scanf("%c",&s);
                if(s == 'Y' || s == 'y'){
                    break;
                }
            }
        }
        else if(!strcmp(command,"diskinfo")){
            show_MBR(Name);
        }
        else if(!strcmp(command,"dir")){
            printf("<Name>      <Type>        <Time>      <Size>\n");
            if(cur_pos == 19)show_root(Name,cur_pos,cur_size);
            else show_normal_entry(Name,cur_pos - 31);
        }
        else if(!strcmp(command,"cd")){
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            char real_name[12];
            scanf("%s",real_name);
            convert_input(real_name);

            struct RootEntry final_res;
            int res = analyse_path(real_name,&cur_pos,&cur_size,&list,Name,&final_res,false);

            if(res == 0){
                bool match = false;
                struct RootEntry* next_entry = match_by_name(cur_entry,real_name,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match);
                if(match && next_entry->DIR_Attr == 0x10){
                    if(cur_entry != root && cur_entry != NULL){
                        free(cur_entry);
                    }
                    int begin = next_entry->DIR_FstClus;
                    cur_entry = Read_Entry(begin,Name,&cur_pos,&cur_size);
                    if(cur_entry != NULL){
                        //print_entry_table(cur_entry,cur_size * Sector_Size);
                        strcpy(list.name[list.size],real_name);
                        if(!strcmp(real_name,"..")){
                            list.size--;
                        }
                        else if(!strcmp(real_name,".")){

                        }
                        else {
                            list.size++;
                        }
                    }
                    else{
                        cur_entry = root;
                        list.size--;
                        if(list.size < 0) list.size = 0;
                    }

                }
                else{
                    printf("No such file or dir\n");
                }       
            }
            else if(res == 1){
                cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            }
            else{
                printf("invalid path.\n");
            }
        }
        else if(!strcmp(command,"type")){
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            char real_name[12];
            scanf("%s",real_name);
            convert_input(real_name);
            bool match = false;
            struct RootEntry* next_entry = match_by_name(cur_entry,real_name,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match);
            if(match && next_entry->DIR_Attr != 0x10){
                char* res = Read_File(next_entry,Name);
                printf("%s\n",res);
                free(res);
            }
            else{
                printf("No such file or directory\n");
            }            
        }
        else if(!strcmp(command,"del")){
            char real_name[12];
            scanf("%s",real_name);
            convert_input(real_name);
            unsigned short* FAT = get_Fat_convert(Name);    
            int pos = 0;
            bool ret = del_file(cur_entry,real_name,cur_size,FAT,&pos);
            if(ret){
                FILE * E5 = fopen(Name,"rb+");
                if(cur_pos == 19){
                    fseek(E5,cur_pos * Sector_Size + pos * Size_Of_A_Root_Entry,SEEK_CUR);
                    unsigned char a[0];
                    a[0] = 0xE5;
                    fwrite(a,1,1,E5);
                    fclose(E5);
                    write_back(Name,FAT);
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
                    write_back(Name,FAT);
                }
            }
            else{
                printf("No such file.\n");
            }
            free(FAT);
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            
        }
        else if(!strcmp(command,"gedit")){
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            
            char input[13];
            memset(input,0,13);
            scanf("%s",input);
            convert_input(input);
            unsigned short* FAT = get_Fat_convert(Name);  
            int ret = creat_new_file_entry(input,cur_entry,cur_pos,cur_size,Name,FAT,&list);
            if(ret == 0){
                printf("File name format error!\n\n");
            }
            else if(ret == -1){
                printf("File of dir exists!\n\n");
            }
            free(FAT);
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
        }
        else if(!strcmp(command,"mkdir")){
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
            unsigned short* FAT = get_Fat_convert(Name);  
            int ret = mkdir(cur_entry,cur_pos,cur_size,Name,FAT);
            if(ret == 0){
                printf("DIR name format error!\n\n");
            }
            else if(ret == -1){
                printf("File or dir exists!\n\n");
            }
            free(FAT);
            free(cur_entry);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
        }
        else if(!strcmp(command,"rd")){
            unsigned short* FAT = get_Fat_convert(Name);  
            char target[12];
            scanf("%s",target);
            int res = rd(cur_entry,cur_pos,cur_size,Name,FAT,target);
            if(res == -1){
                printf("No such dir\n");
            }
            else if(res == 0){
                printf("The dirctory is not empty.\n");
            }
            free(FAT);
            cur_entry = Read_Entry(cur_pos - 31,Name, &cur_pos,&cur_size);//更新目录
        }
        else if(!strcmp(command,"help")){
            printf("\n\tdir       --check current directory.\n");
            printf("\tcd        --enter a new directory.\n");
            printf("\tmkdir     --creat a new directory.\n");
            printf("\trd        --delete a directory.\n");
            printf("\tgedit     --Creat a new file or edit a existed file.\n");
            printf("\tdel       --delete a file.\n");
            printf("\tcp        --copy a file to a directory.\n");
            printf("\tquit      --end the program.\n");
            printf("\ttree      --list the all directories in this directory.\n\n");
        }
        else if(!strcmp(command,"tree")){
            unsigned short* FAT = get_Fat_convert(Name);
            tree(cur_pos,cur_size,0,Name,FAT);
            free(FAT);
        }
        else if(!strcmp(command,"cp")){
            unsigned short*FAT = get_Fat_convert(Name);
            char source[20];
            char dest[20];
            scanf("%s",dest);
            scanf("%s",source);

            int res = cp(dest,source,Name,FAT);
            free(FAT);
        }
        else if(!strcmp(command,"quit")){
            break;
        }
        else{
            printf("Invaild command, type help to get help.\n");
        }
    }
    
    system("pause");
}
