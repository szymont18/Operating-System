//
// Created by szymek on 06.03.23.
//


#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "classifier.h"
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>


int main(){


#ifdef DYNAMIC
    void * handle1 = dlopen("liblibrary.so", RTLD_LAZY);
    if(!handle1){
        puts("CAN NOT OPEN LIBRARY");
        return -1;
    }

    Table *(*create_Table)(int);
    create_Table = dlsym(handle1,"create_Table");

    int (*add_block)(Table * , char *);
    add_block = dlsym(handle1,"add_block");

    Block * (*get_block)(Table *, int);
    get_block = dlsym(handle1,"get_block");

    int (*delete_block)(Table *, int);
    delete_block = dlsym(handle1,"delete_block");

    int (*delete_table)(Table *);
    delete_table = dlsym(handle1,"delete_table");

    void *handle2 = dlopen("libclassifier.so", RTLD_LAZY);
    if(!handle2){
        puts("CAN NOT OPEN CLASSIFIER");
        return 2;
    }
    int (*classify)(char * first, char * second , char * third, char * fourth);
    classify = dlsym(handle2,"classify");
#endif

    puts("-------------WELCOME TO CLI------------------");
    puts("Type 'help' to know the available commands");

    char command[512];

    Table * table = NULL;

    struct tms start, end;
    clock_t start_time, end_time;

    while(fgets(command, 512, stdin) != NULL){

        start_time = clock();
        times(&start);
        // Parse command
        char *first, *second, *third, *fourth;

        first = strtok(command, " ");
        second = strtok(NULL, " ");
        third = strtok(NULL, " ");
        fourth = strtok(NULL, " ");

        //Pointer to the table ( without command 'init' it is just NULL )

        int command_id = classify(first, second, third, fourth);
        printf("%s %s %s ", first, second, third);

        switch (command_id) {
            case 0:
                puts("List of available instructions: ");
                puts("1) 'init' size - create an array of size (int)");
                puts("2) 'count' filename - counting words and lines in the file filename saving the result in the table");
                puts("3) 'show' index - displaying the contents of an array with index index");
                puts("4) 'delete index' index - deletion of blocks with index index");
                puts("5) 'destroy' - delete table from memory");
                puts("6) 'exit' - end the program");
                break;

            case 1:
                if (table != NULL){
                    puts("The array is already initialized. Destroy it with 'destroy' to create a new one");
                    break;
                }

                int capacity = atoi(second);
                table = create_Table(capacity);
                puts("Table created");
                break;

            case 2:
                if (table == NULL){
                    puts("Error: No array has been initialized yet");
                    break;
                }

                int error1 = add_block(table, second);

                if (error1 == 5) puts("Error: There is no place in table for this record");
                else if(error1 == 2) puts("Error: The file does not exist");
                else puts("Block added");
                break;

            case 3:
                if (table == NULL){
                    puts("Error: No array has been initialized yet");
                    break;
                }

                int index1 = atoi(second);
                Block * block = get_block(table, index1);
                puts(block->wc);
                break;

            case 4:
                if (table == NULL){
                    puts("Error: No array has been initialized yet");
                    break;
                }

                int index = atoi(third);
                int error2 = delete_block(table, index);

                if (error2 == INDEX_OUT_OF_BOUNDARY) puts("Error: Index out of boundary");
                else(puts("Block deleted"));
                break;

            case 5:
                if (table == NULL){
                    puts("Error: No array has been initialized yet");
                    break;
                }

                delete_table(table);
                table = NULL;
                puts("Table destroyed");
                break;

            case 6:
                goto end;
                break;

            default:
                puts("There is no such command");

        }
        end_time = clock();
        times(&end);

        double clocks_per_second = (double) sysconf(_SC_CLK_TCK);
        printf("----- Real Time: %.8f ms ----- User Time: %.8f ms ----- System Time: %.8f ms -----\n",
               ((double) end_time - (double) start_time) * 1000 / CLOCKS_PER_SEC,
               ((double) end.tms_utime - (double) start.tms_utime) * 1000 / clocks_per_second,
               ((double) end.tms_stime - (double) start.tms_stime) * 1000 / clocks_per_second);

        puts("_________________________________________________________________________");
    }

    end:
        puts("PROGRAM CLOSED");

#ifdef DYNAMIC
    dlclose(handle1);
    dlclose(handle2);
#endif
}