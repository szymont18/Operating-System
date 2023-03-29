//
// Created by szymek on 28.03.23.
//
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include "string.h"
#include "stdlib.h"

#define COUNT 1
#define TIME 2
#define REQUEST 3
#define TIME_SEC 4
#define FINISH 5


void handler(int signum);

int main(int argc, char ** argv){
    if(argc < 3){
        printf("Error: Missed %d arguments\n", 3 - argc);
        printf("Correct format: ./sender catcher_pid option1 [other_options]\n");
        return -1;
    }

    int catcher_pid = 0;
    if ((catcher_pid = atoi(argv[1])) == 0){
        printf("Error: Incorrect format\n");
        printf("Correct format: ./sender catcher_pid option1 [other_options]\n");
        return -1;
    }

    int option_number;
//    Sigset
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);

//    Sigaction
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = handler;

    union sigval value;

    if (sigaction(SIGUSR1, &sact, NULL) != 0)
        perror("SIGACTION ERROR");

    for(int i = 2; i < argc; i++){
        option_number = atoi(argv[i]);

        value.sival_int = option_number;
        printf("Option number: %d\n", option_number);
        sigqueue(catcher_pid, SIGUSR1, value);
        sigsuspend(&sigset);
    }
}

void handler(int signum){
    printf("Confirmation provided\n");
}