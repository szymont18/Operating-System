#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "string.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char ** argv){

    if (argc < 5){
        printf("ERROR: Missed %d arguments\n", 5 - argc);
        return -1;
    }

    clock_t start_time, end_time;
//    Time File

    FILE * time_file = fopen("pomiar_zad_1.txt", "a");


//    LIB

    start_time = clock();

    FILE * file = fopen(argv[3], "r");

    if (file == NULL){
        printf("ERROR: Can not open file '%s' \n", argv[3]);
        return -2;
    }

    FILE * write_file = fopen(argv[4], "w+");

    if(write_file == NULL){
        printf("ERROR: Can not open file '%s' \n", argv[4]);
        return -2;
    }

    char buffer[1];

    while(fread(&buffer, 1, 1, file) > 0){
        if (buffer[0] == *argv[1]) buffer[0] = *argv[2];

        if (fwrite(&buffer, 1, 1, write_file) == 0){
            printf("ERROR: Can not write to file '%s'", argv[4]);
            return -3;
        }
    }

    fclose(file);
    fclose(write_file);
    end_time = clock();

    char comment[256];
    int count = snprintf(comment, 256, "C LIBRARY FUNCTIONS - %.8f ms \n",((double) end_time - (double) start_time) * 1000 / CLOCKS_PER_SEC);

    fwrite(comment, 1, count, time_file);

    fclose(time_file);

    return 0;

}