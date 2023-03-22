//
// Created by szymek on 13.03.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "string.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char ** argv) {

    if (argc < 5) {
        printf("ERROR: Missed %d arguments\n", 5 - argc);
        return -1;
    }

    clock_t start_time, end_time;
//    Time File
    FILE *time_file = fopen("pomiar_zad_1.txt", "a");


    //    SYS

    start_time = clock();
    int file_sys = open(argv[3], O_RDONLY);
    if (file_sys == -1) {
        printf("ERROR: Can not open file '%s\n'", argv[3]);
        return -1;
    }

    int write_file_sys = open(argv[4], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (write_file_sys == -1) {
        printf("ERROR: Can not open file '%s\n'", argv[4]);
        return -2;
    }
    char buffer[1];

    while (read(file_sys, &buffer, 1) == 1) {
        if (buffer[0] == *argv[1]) buffer[0] = *argv[2];

        if (write(write_file_sys, &buffer, 1) == -1) {
            printf("ERROR: Can not write to file '%s'", argv[4]);
            return -3;
        }
    }

    end_time = clock();
    char comment[256];
    int count = snprintf(comment, 256, "SYS FUNCTIONS - %.8f ms \n",
                         ((double) end_time - (double) start_time) * 1000 / CLOCKS_PER_SEC);
    fwrite(comment, 1, count, time_file);

    close(write_file_sys);
    close(file_sys);
    return 0;
}