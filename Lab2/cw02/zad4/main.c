#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>

long long total_size = 0LL;

int fn(const char * fpath, const struct stat * f_stat, int flag){
    if(flag == FTW_NS){
        printf("ERROR: Can not get struct stat\n");
        return -1;
    }
    if (flag != FTW_F) return 0;  //For every file (not dir)

    printf("FILE: %s  SIZE: %jd bytes\n", fpath, f_stat->st_size);
    total_size += (long long) f_stat->st_size;
    return 0;
}


int main(int argc, char ** argv){
    if (argc < 2){
        printf("ERROR: Missed %d arguments\n", 2 - argc);
        return -1;
    }

    char * dir_name = argv[1];


    if(ftw(dir_name, fn, FOPEN_MAX) == -1){
        printf("ERROR: Can not travel through direction '%s'\n", dir_name);
        return -1;
    }
    printf("Total size: %lld\n", total_size);
    return 0;
}