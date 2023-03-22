#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char ** argv) {

    if (argc < 2) {
        printf("ERROR: You missed %d arguments\n", 2 - argc);
        return -1;
    }

    pid_t root_pid = getpid();
    int boundary;
    if (strcmp(argv[1], "0") == 0) boundary = 0;

    else {
        boundary = atoi(argv[1]);
        if (boundary <= 0 ){
            printf("ERROR: First argument have to represent non-negative number\n");
            return -1;
        }
    }

    int *stat_loc = malloc(sizeof (int *));

    for (int i = 0; i < boundary; i++) {
        if (root_pid == getpid()) {
            fork();
            wait(stat_loc);
        }
        else break;
    }

    if(root_pid != getpid()){
        printf("My pid: %d, Parent pid: %d\n", getpid(), getppid());
    }

    free(stat_loc);

    if (root_pid == getpid()){
        printf("Argv[1] = %s\n", argv[1]);
    }
    return 0;
}