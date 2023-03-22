#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>


int main(){
    DIR * current_dir = opendir(".");

    if (current_dir == NULL){
        printf("ERROR: Can not open current direction\n");
        return -1;
    }

    long long total_size = 0LL;
    struct dirent * file;
    struct stat * file_stat = malloc(sizeof (struct stat));
    do {
        file = readdir(current_dir);

        stat(file->d_name, file_stat);

//        File is Dir
        if(S_ISDIR(file_stat->st_mode)) continue;

//        File is not Dir

        if(errno != 0){
            printf("ERROR: Can not access file in current directory\n");
            return -1;
        }
        printf("FILE NAME: %s, SIZE: %jd bytes\n", file->d_name, file_stat->st_size);

        total_size = total_size + (long long) file_stat->st_size;
    }   while(file != NULL);

    printf("Total size = %lld\n", total_size);

    free(file_stat);
    return 0;
}