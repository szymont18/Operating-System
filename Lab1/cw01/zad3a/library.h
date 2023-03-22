#ifndef Z1_LIBRARY_H
#define Z1_LIBRARY_H

#include <bits/types/FILE.h>

#define SUCCESS 1
#define FAILURE 0
#define FILE_EXIST 1
#define FILE_NOT_EXIST 2
#define FILE_CAN_NOT_OPEN 3
#define TMP_FILE_NOT_REMOVED 4
#define INDEX_OUT_OF_BOUNDARY 5

typedef struct Block{
    char * wc;
}Block;

typedef struct Table{
    Block ** table;
    int capacity;
    int current_capacity;
}Table;

Table * create_Table(int capacity);

int if_file_exist(char * filename);

int add_block(Table * table, char * filename);

Block * get_block(Table * table, int ind);

void delete_wc(Block * block);

int delete_block(Table * table, int ind);

int delete_table(Table * table);

#endif //Z1_LIBRARY_H
