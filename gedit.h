#pragma once
#include "FAT.h"
#include "MBR.h"
#include "RootEntry.h"
#include "Macro.h"
#include "Find_entry.h"
#include "Find_file.h"
#include "path.h"


void get_time_and_convert(short*a,short*b){
    time_t rawtime;  //时间类型，由time.h中定义
    struct tm *timeinfo;  //关于时间的结构体类型，在time.h中定义
    timeinfo = (struct tm*)malloc(sizeof(struct tm));
    time(&rawtime);  //获取时间，以秒计，从1970年1月1日00：00：00起算，存于rawtime
    timeinfo = localtime(&rawtime);  //转换为当地时间，存在结构体 struct tm中
    unsigned int year = timeinfo->tm_year- 80;
    unsigned int mon = timeinfo->tm_mon + 1;
    unsigned int day = timeinfo->tm_mday;
    unsigned int hour = timeinfo->tm_hour;
    unsigned int min = timeinfo->tm_min;
    *a = 0,*b = 0;
    year = year << 9;
	*a = *a + year;
	mon = mon << 5;
	*a += mon;
	*a += day;
	hour = hour << 11;
	*b += hour;
	min = min << 5;
	*b += min;
    free(timeinfo);
}

void clear(){
    system("cls");   
}
void edit(char*file,int*len){
    fflush(stdin);
    clear();
    printf("%s",file);
    char c;
    while(1){
        c = getch();
        if(c == 8){
            file[(*len) - 1] = 0;
            (*len)--;
            if((*len) < 0) (*len) = 0;
            clear();
            printf("%s",file);
        }
        else if(c == 13){
            printf("\n");
            file[(*len)] = '\n';
            (*len)++;            
        }
        else if(c == 27 || c == 26)break;
        else{
            file[(*len)] = c;
            (*len)++;
            printf("%c",c);
        }
    }
}
void get_msg(char*s,int*len){
    fflush(stdin);
    char c;
    *len = 0;
    while(1){
        c = getchar();
        if(c == EOF || c == 26)break;
        s[(*len)++] = c;
    }
}
bool convert_input_string_to_entry_name(char input[13],char dest[11]){
    bool flag = false;
    int pos = 0;
    for(int i = 0;i < 13;i++){
        if(input[i] == '.'){
            flag = true;
            pos = i;
            break;
        }
    }
    int len = strlen(input);
    if(!flag){
        if(len > 8)return false;
        for(int i = 0;i < 11;i++){
            dest[i] = 0x20;
        }
        for(int i = 0;i < len;i++){
            dest[i] = input[i];
        }
        return true;
    }
    if(pos <= len - 5 || pos > 8){
        return false;
    }
    for(int i = 0;i < 11;i++){
        dest[i] = 0x20;
    }
    
    for(int i = 0;i < pos;i++){
        dest[i] = input[i];
    }
    for(int i = 8,j = pos + 1;j < len;i++,j++){
        dest[i] = input[j];
    }
    return true;
}


//接下来的工作，找到空簇，然后根据大小创建文件
//这个代码在创建目录的地方也可以用
bool find_FAT_pos(unsigned short*FAT,int size,struct file_cluster*res){
    int num_of_sector_needed = size / Sector_Size;
    if(size % Sector_Size)num_of_sector_needed++;
    bool exit = false;
    int begin;
    bool enough = false;
    for(int i = 2;i < Num_Of_Fat;i++){  
        if(FAT[i] == 0){
            begin = i;
            exit = true;
            break;
        }
    }
    if(exit == false){
        return false;
    }
    res->count = 1;
    res->clusters[0] = begin;
    for(int i = begin + 1;i < Num_Of_Fat;i++){
        if(res->count == num_of_sector_needed)return true;
        if(FAT[i] == 0){
            res->clusters[res->count++] = i;
        }
    }
    if(res->count == num_of_sector_needed)return true;
    if(res->count < num_of_sector_needed)return false;
}


void write_a_new_file_to_FAT(unsigned short*FAT,struct file_cluster res,char*img_name){
    for(int i = 0;i < res.count - 1;i++){
        FAT[res.clusters[i]] = res.clusters[i + 1];
    }
    FAT[res.clusters[res.count - 1]] = 0xFFF;
    write_back(img_name,FAT);
}



void wirte_file_to_sector(struct file_cluster res,unsigned char*msg,int len,char*img_name){
    FILE* target = fopen(img_name,"rb+");
    for(int i = 0;i < res.count - 1;i++){
        fseek(target,(res.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fwrite(msg + i * Sector_Size,1,Sector_Size,target);
    }
    fseek(target,(res.clusters[res.count - 1] + 31) * Sector_Size,SEEK_CUR);
    fwrite(msg + (res.count - 1) * Sector_Size,1,len % Sector_Size,target);
    fclose(target);
}

void clean(struct file_cluster res,char*img_name){
    FILE* target = fopen(img_name,"rb+");
    char empty[Sector_Size];
    memset(empty,0,Sector_Size);
    for(int i = 0;i < res.count;i++){
        fseek(target,(res.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fwrite(empty,1,Sector_Size,target);
    }
    fclose(target);    
}
void wirte_entry_to_sector(struct file_cluster res,unsigned char*msg,int len,char*img_name){

    FILE* target = fopen(img_name,"rb+");
    for(int i = 0;i < res.count - 1;i++){
        fseek(target,(res.clusters[i] + 31) * Sector_Size,SEEK_CUR);
        fwrite(msg + i * Sector_Size,1,Sector_Size,target);
    }
    fseek(target,(res.clusters[res.count - 1] + 31) * Sector_Size,SEEK_CUR);
    fwrite(msg + (res.count - 1) * Sector_Size,1,len % Sector_Size,target);
    fclose(target);
}
//返回0说明名字不符合规则，返回-1说明文件名已经出现过了，return 1说明成功创建,返回2说明无效路径
int creat_new_file_entry(char*input,unsigned char*cur_entry,int cur_pos,int cur_size,char*img_name,unsigned short*FAT,struct entry_detial*list){
    struct RootEntry final_res;
    int find_file = analyse_path(input,&cur_pos,&cur_size,list,img_name,&final_res,true);
    free(cur_entry);
    cur_entry = Read_Entry(cur_pos - 31,img_name, &cur_pos,&cur_size);//更新目录

    if(find_file == -1)return 2;
    if(find_file == 1)return -1;
    
    if(new_file_name_match(cur_entry,cur_size,input)){
        bool match;
        int pos = 0;
        struct RootEntry*target = match_by_name_for_del(cur_entry,input,cur_size * Sector_Size / Size_Of_A_Root_Entry,&match,&pos);


        if(target == NULL)return -1;//说明存在文件名相同，扩展名不同的文件
        if(target->DIR_Attr == 0x10)return -1;

        struct file_cluster file_cluster = Build_File(FAT,target->DIR_FstClus);
        char* edit_string =  Read_File(target,img_name);
        int len = target->DIR_FileSize;
        char res[Max_File_len];
        memcpy(res,edit_string,len);
        free(edit_string);
        edit(res,&len);
        printf("\n");
        wirte_file_to_sector(file_cluster,res,len,img_name);

        short day,time;
        get_time_and_convert(&day,&time);

        target->DIR_WrtDate = day;
        target->DIR_WrtTime = time;
        target->DIR_FileSize = len;


        //将目录项写回文件
        FILE* wirte_entry = fopen(img_name,"rb+");
        fseek(wirte_entry,cur_pos * Sector_Size + pos * Size_Of_A_Root_Entry,SEEK_CUR);
        fwrite(target,1,Size_Of_A_Root_Entry,wirte_entry);

        fclose(wirte_entry);



        free(target);

        return 1;

    }
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

    char*msg = (char*)malloc(Max_File_len);
    int len = 0;
    get_msg(msg,&len);


    struct file_cluster res;
    find_FAT_pos(FAT,len,&res);
    write_a_new_file_to_FAT(FAT,res,img_name);
    wirte_file_to_sector(res,msg,len,img_name);


    new_entry.DIR_FstClus = res.clusters[0];
    new_entry.DIR_Attr = 0x20;
    new_entry.DIR_FileSize = len;


    short day,time;
    get_time_and_convert(&day,&time);
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

//返回0说明名字不符合规则，返回-1说明文件名已经出现过了，return 1说明成功创建,返回2说明无效路径
int creat_new_file_entry_for_cp(char*input,unsigned char*cur_entry,int cur_pos,int cur_size,char*img_name,unsigned short*FAT,char*msg,int len){
    free(cur_entry);
    cur_entry = Read_Entry(cur_pos - 31,img_name, &cur_pos,&cur_size);//更新目录

    if(new_file_name_match(cur_entry,cur_size,input)){
        return -1;
    }
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

    struct file_cluster res;
    find_FAT_pos(FAT,len,&res);
    write_a_new_file_to_FAT(FAT,res,img_name);
    wirte_file_to_sector(res,msg,len,img_name);


    new_entry.DIR_FstClus = res.clusters[0];
    new_entry.DIR_Attr = 0x20;
    new_entry.DIR_FileSize = len;


    short day,time;
    get_time_and_convert(&day,&time);
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
