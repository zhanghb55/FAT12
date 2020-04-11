#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define Sector_Size 512
#define File_Size 1474560
#define Num_Of_Fat 3072 

#define Sector_Of_MBR 1
#define Num_Of_Fat_Table 2
#define Sector_Of_Fat_Table 9
#define Sector_Of_Root_Entry 14 
#define Size_Of_A_Root_Entry 32 
#define Num_Of_Fats 3072 

#define bool char
#define true 1
#define false 0

#define Max_File_len 5120
