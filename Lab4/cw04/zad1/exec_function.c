//
// Created by szymek on 27.03.23.
//
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "string.h"
#include "stdlib.h"
#include <ctype.h>

#define IGNORE 1
#define HANDLER 2
#define MASK 3
#define PENDING 4
#define ERROR -1

int get_option(char * string_option);

int main(int argc, char ** argv){
    int option = get_option(argv[1]);
    int signal_number = SIGUSR1;
    if(argc == 3){
        if((signal_number = atoi(argv[2])) == 0){
            puts("Error: Second parameter must be non negative integer");
            return -1;
        }
    }

    sigset_t pending_signals;

    switch (option) {
        case IGNORE:
            raise(signal_number);
            printf("CHILD (EXEC) PROCESS - IGNORE SIGNAL '%s'\n", sys_siglist[signal_number]);
            break;
        case MASK:
            raise(signal_number);
            printf("CHILD (EXEC) PROCESS - MASK SIGNAL '%s'\n", sys_siglist[signal_number]);
            break;
        case PENDING:
            sigpending(&pending_signals);
            if(sigismember(&pending_signals, signal_number)) printf("CHILD (EXEC) PROCESS - PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
            else printf("CHILD (EXEC) PROCESS - NOT! PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
            break;
        case HANDLER:
            printf("CHILD (EXEC) PROCESS\n");
            raise(signal_number);
            break;

    }
    return 0;
}


int get_option(char * string_option){
    if (strcmp("IGNORE", string_option) == 0) return IGNORE;
    if(strcmp("HANDLER", string_option) == 0) return HANDLER;
    if(strcmp("MASK", string_option) == 0) return MASK;
    if(strcmp("PENDING", string_option) == 0) return PENDING;
    return ERROR;

}