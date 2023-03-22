#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



void str_rev(char * string, char * reversed, int size){
    for(int i = size - 1; i >= 0; i--){
        reversed[size - i - 1] = string[i];
    }
}

int main(int argc, char ** argv){

    if (argc < 3){
        printf("ERROR: Missed %d arguments\n", 3 - argc);
        return -1;
    }

    FILE * time_file = fopen("pomiar_zad_2.txt", "a");

    if(time_file == NULL){
        printf("ERROR: Can not open time_file");
        return -1;
    }

    clock_t start_time, end_time;


//    Reading by blocks

    FILE *read = fopen(argv[1], "rb");
    if (read == NULL) {
        printf("ERROR: Can not open file '%s'\n", argv[1]);
        return -1;
    }

    start_time = clock();

    FILE * write_by_block = fopen(argv[2], "w");

    if(write_by_block == NULL){
        printf("ERROR: Can not open file '%s'\n", argv[2]);
        return -1;
    }

    int i = 0;
    fseek(read, -i, SEEK_END);
    int size = ftell(read);
    int chunk = 1024;
    char buffer[1024];

    while(i < size){
        i += chunk;
        if (i > size) {
            chunk = size - (i - chunk);
            i = size;
        }
        fseek(read, -i, SEEK_END);

        int n = fread(buffer, sizeof (char), chunk, read);
        if (n == 0){
            printf("ERROR: Can not read from file '%s'", argv[2]);
            return -2;
        }

        char reversed[n];
        str_rev(buffer, reversed, n);

        if(fwrite(reversed, sizeof (char), n, write_by_block) == 0){
            printf("ERROR: Can not write to file '%s'", argv[2]);
            return -2;
        }
    }
    fclose(write_by_block);
    end_time = clock();
    char comment[256];
    int count = snprintf(comment, 256, "Reading BLOCKS - %.8f ms \n",((double) end_time - (double) start_time) * 1000 / CLOCKS_PER_SEC);
    fwrite(comment, 1, count, time_file);

    fclose(read);
    fclose(time_file);

    return 0;
}