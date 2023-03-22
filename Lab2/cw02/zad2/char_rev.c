//
// Created by szymek on 13.03.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char ** argv) {

    if (argc < 3) {
        printf("ERROR: Missed %d arguments\n", 3 - argc);
        return -1;
    }

    FILE *time_file = fopen("pomiar_zad_2.txt", "a");

    if (time_file == NULL) {
        printf("ERROR: Can not open time_file");
        return -1;
    }

    clock_t start_time, end_time;
    start_time = clock();

//    Reading one character at time
    FILE *read = fopen(argv[1], "rb");
    if (read == NULL) {
        printf("ERROR: Can not open file '%s'\n", argv[1]);
        return -1;
    }
    FILE *write_by_char = fopen(argv[2], "w");

    int i = 0;
    fseek(read, -i, SEEK_END);
    int size = ftell(read);
    char letter;

    while (i < size) {
        i += 1;
        fseek(read, -i, SEEK_END);
        if (fread(&letter, 1, 1, read) == 0) {
            printf("ERROR: Can not read from file '%s'", argv[1]);
            return -2;
        }

        if (fwrite(&letter, 1, 1, write_by_char) == 0) {
            printf("ERROR: Can not write to file '%s", argv[2]);
            return -2;
        }
    }
    fclose(write_by_char);
    end_time = clock();
    char comment[256];
    int count = snprintf(comment, 256, "Reading ONE CHARACTER - %.8f ms \n",
                         ((double) end_time - (double) start_time) * 1000 / CLOCKS_PER_SEC);

    fwrite(comment, 1, count, time_file);
    fclose(time_file);
    fclose(read);
    return 0;
}