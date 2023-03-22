#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char ** argv){
    setbuf(stdout, NULL);
    if (argc < 2){
        printf("ERROR: Missed %d arguments\n", 2 - argc);
        return -1;
    }

    printf("Program name: %s ", argv[0]);
    pid_t child_pid = fork();

    if (child_pid == 0){
        int error = execl("/bin/ls", "ls", argv[1], NULL);
        if (error == -1){
            printf("ERROR: Can not do execl command\n");
            return -1;
        }
    }
    int *stat_loc = malloc(sizeof (int *));
    wait(stat_loc);
    free(stat_loc);
    return 0;
}