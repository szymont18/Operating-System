#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/wait.h>


#define SUCCESS 1
#define FAILURE 0

const char dot[] = ".";
const char dot2[] = "..";

int check_beginning(FILE * file, char * pattern){
    int length = strlen(pattern);
    char buffer[length];
    int count = fread(buffer, 1, length, file);
    buffer[length] = 0;
    if (count != length) return FAILURE;

    if(strcmp(pattern, buffer) == 0) return SUCCESS;

    return FAILURE;
}

void print_res(char *file_name){
    printf("PATH: %s. PID: %d\n", file_name, getpid());
}

void search_file(char * dir_name, char * pattern){
    DIR * main_dir = opendir(dir_name);

    if(main_dir == NULL){
        perror("ERROR");
        exit(1);
    }

    int *stat_loc = malloc(sizeof (int *));
    struct dirent * file;
    struct stat file_stat;
    char buffer[PATH_MAX + 1];
    while((file = readdir(main_dir)) != NULL) {

        if(strcmp(file->d_name, dot) == 0 || strcmp(file->d_name, dot2) == 0) continue;
        stat(file->d_name, &file_stat);

        sprintf(buffer, "%s/%s", dir_name, file->d_name);
        if (file->d_type == DT_DIR){

            pid_t new_process = fork();
            if(new_process == 0) {
                search_file(buffer, pattern);
                goto close;
            }
        }
        else {

            FILE * file_handle = fopen(buffer, "r");
            if (file_handle == NULL) {
                perror("ERROR");
                exit(1);
            }

            if (check_beginning(file_handle, pattern) == SUCCESS) print_res(buffer);
            fclose(file_handle);
        }

    }
    close:
    wait(stat_loc);
    free(stat_loc);
    closedir(main_dir);
}
int main(int argc, char ** argv){
    if (argc < 3){
        printf("ERROR: Missed %d arguments \n", 3 - argc);
        return -1;
    }

    if(strlen(argv[2]) > 255){
        printf("ERROR: The length of second argument must be less than 256\n");
        return -1;
    }

    search_file(argv[1], argv[2]);

    return 0;
}