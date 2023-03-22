#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"

Table * create_Table(int capacity){
    Table * new_table = malloc(sizeof (Table));
    new_table -> table = calloc(capacity, sizeof (Block *));
    new_table ->capacity = capacity;
    new_table ->current_capacity = 0;
    return new_table;
}

int if_file_exist(char * filename){
    if (access(filename, F_OK) == 0) {
        return FILE_EXIST;
    } else {
        return FILE_NOT_EXIST;
    }
}


int add_block(Table * table, char * filename){
    if (table -> capacity == table -> current_capacity) return INDEX_OUT_OF_BOUNDARY;
    if (if_file_exist(filename) == FILE_NOT_EXIST) return FILE_NOT_EXIST;

    char tmp_filename[] = "/tmp/tmp_file_wc";
    mkstemp(tmp_filename);


    char command[256];
    snprintf(command,256, "wc %s > /tmp/tmp_file_wc", filename);

    system(command);
    FILE *tmp_file = fopen(tmp_filename, "rb");
    if (tmp_file == NULL) return FILE_CAN_NOT_OPEN;

    fseek(tmp_file, 0L, SEEK_END);
    long size = ftell(tmp_file);
    fseek(tmp_file, 0L, SEEK_SET);


    Block *block = malloc(sizeof (Block));

    block -> wc = malloc(sizeof (char) * size);

    fgets(block->wc, (int) size, tmp_file);

    table->table[table->current_capacity] = block;


    table -> current_capacity += 1;

    if(remove("/tmp/tmp_file_wc") == 0){
        return SUCCESS;
    }

    else return TMP_FILE_NOT_REMOVED;
}

Block * get_block(Table * table, int ind){
    if (ind >= table -> current_capacity || ind < 0) {
        Block *block = malloc(sizeof (Block));
        block -> wc = "Error: Index out of range";
        return block;
    }
    Block * res = table -> table[ind];
    return res;
}

void delete_wc(Block * block){
    free(block -> wc);
}


int delete_block(Table * table, int ind){
    if (ind > table -> current_capacity || ind < 0) return INDEX_OUT_OF_BOUNDARY;

    Block * block = table -> table[ind];

    delete_wc(block);
    free(block);

    for(int i = ind + 1; i < table -> current_capacity; i++){
        table -> table[i - 1] = table -> table[i];
    }
    table -> current_capacity--;
    return SUCCESS;
}

int delete_table(Table * table){
    int current_capacity = table ->current_capacity;
    for (int i = current_capacity - 1; i >= 0; i--){
        delete_block(table, i);
    }
    free(table->table);
    free(table);
    return SUCCESS;
}

