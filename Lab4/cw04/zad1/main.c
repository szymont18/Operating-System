#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include "stdlib.h"
#include "sys/wait.h"

#define IGNORE 1
#define HANDLER 2
#define MASK 3
#define PENDING 4
#define ERROR -1

int get_option(char * string_option);
void print_option_list();
void ignore_test(int signal_number, char ** argv);
void handler(int signal_number);
void handle_test(int signal_number, char ** argv);
void mask_test(int signal_number, char ** argv);
void pending_test(int signal_number, char ** argv);

int main(int argc, char ** argv) {
    if (argc < 2){
        printf("Error: Missed %d arguments\n", 2 - argc);
        print_option_list();
        return -1;
    }
    int option ;
    if ((option= get_option(argv[1])) < 0){
        printf("Error: There is no option '%s' ", argv[1]);
        print_option_list();
        return -1;
    }
    int signal_number = SIGUSR1;

    if(argc == 3){
        if((signal_number = atoi(argv[2])) == 0){
            puts("Error: Second parameter must be non negative integer");
            return -1;
        }
    }

    sigset_t mask;

    printf("SIGNAL = %s\n __________________________\n", sys_siglist[signal_number]);
    printf("Option: '%s'\n", argv[1]);
    switch (option) {
        case IGNORE:
            signal(signal_number, SIG_IGN);
            ignore_test(signal_number, argv);
            break;

        case HANDLER:
            signal(signal_number, handler);
            handle_test(signal_number, argv);
            break;

        case MASK:
            sigemptyset(&mask);
            sigaddset(&mask, signal_number);

            sigprocmask(SIG_SETMASK, &mask, NULL);
            mask_test(signal_number, argv);
            break;

        case PENDING:
            sigemptyset(&mask);
            sigaddset(&mask, signal_number);

            sigprocmask(SIG_SETMASK, &mask, NULL);
            pending_test(signal_number, argv);
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


void print_option_list(){
    puts("Correct format is: ./main option1 [signal number]");
    puts("List of options: \n 1)IGNORE - ignore the signal \n 2)HANDLER - install the handler \n "
         "3)MASK - mask signal\n 4)PENGING - check if pending signal is visible\n");
}


void ignore_test(int signal_number, char ** argv){
    pid_t child_pid = fork();

    if(child_pid > 0){
        raise(signal_number);
        printf("PARENT PROCESS - IGNORE SIGNAL '%s'\n", sys_siglist[signal_number]);
        execv("./exec_function", argv);
    }
    else{
        raise(signal_number);
        printf("CHILD (FORK) PROCESS - IGNORE SIGNAL '%s'\n", sys_siglist[signal_number]);
    }
}

void handler(int signal_number){
    printf("SIGNAL HANDLED '%s' \n", sys_siglist[signal_number]);
}

void handle_test(int signal_number, char ** argv){
    pid_t child_pid = fork();

    if(child_pid > 0){
        printf("PARENT PROCESS - ");
        raise(signal_number);
        execv("./exec_function", argv);
    }
    else{
        printf("CHILD (FORK) PROCESS - ");
        raise(signal_number);
    }
    if(child_pid > 0) waitpid(child_pid, NULL,WNOHANG);
}

void mask_test(int signal_number, char ** argv){
    pid_t child_pid = fork();

    if(child_pid > 0){
        raise(signal_number);
        printf("PARENT PROCESS - MASK SIGNAL '%s'\n", sys_siglist[signal_number]);
        execv("./exec_function", argv);
    }
    else{
        raise(signal_number);
        printf("CHILD (FORK) PROCESS - MASK SIGNAL '%s'\n", sys_siglist[signal_number]);
    }
}

void pending_test(int signal_number, char ** argv){

    raise(signal_number);
    pid_t child_pid = fork();

    sigset_t pending_signals;

    if(child_pid > 0){
        sigpending(&pending_signals);
        if (sigismember(&pending_signals, signal_number) == 1) printf("PARENT PROCESS - PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
        else printf("PARENT PROCESS - NOT! PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
        execv("./exec_function", argv);
    }
    else{
        sigpending(&pending_signals);

        if (sigismember(&pending_signals, signal_number) == 1) printf("CHILD (FORK) PROCESS - PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
        else printf("CHILD (FORK) PROCESS - NOT! PENDING SIGNAL '%s'\n", sys_siglist[signal_number]);
    }

}