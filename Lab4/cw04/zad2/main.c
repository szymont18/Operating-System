#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "string.h"
#include "stdlib.h"
#include "sys/wait.h"


#define SIGINFO 1
#define NOCLDSTOP 2
#define RESETHAND 3
#define ERROR -1

void print_option_list();
int get_option(char * string_option);
void siginfo_handler(int signo, siginfo_t *info, void *other);

int main(int argc, char ** argv){
    if (argc < 2){
        printf("Error: Missed %d arguments\n", 2 - argc);
        print_option_list();
        return -1;
    }
    int option = get_option(argv[1]);
    if (option == ERROR){
        printf("Error: Incorrect argument\n");
        print_option_list();
        return -1;
    }
    struct sigaction act;
    pid_t child_pid;

    printf("OPTION: '%s'\n#############################\n", argv[1]);
    switch (option) {
        case SIGINFO:
            act.sa_sigaction = siginfo_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = SA_SIGINFO;
            sigaction(SIGUSR1, &act, NULL);

//            Without sigval
            raise(SIGUSR1);

            if (fork() == 0) {
                raise(SIGUSR1);
                exit(0);
            }
//            With sigval
            union sigval value;
            value.sival_int = 118;
            sigqueue(getpid(), SIGUSR1, value);
            break;

        case NOCLDSTOP:
            act.sa_sigaction = siginfo_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = SA_SIGINFO;
            sigaction(SIGCHLD, &act, NULL);


            child_pid = fork();
            if(child_pid == 0){
                while(0 == 0);
            }
            else{
                puts("SA_NOCLDSTOP not ative");
                sleep(1);
                puts("Sending SIGSTOP to child process");
                puts("");
                kill(child_pid, SIGSTOP);
                sleep(1);

                puts("Sending SIGCONT to child process");
                puts("");
                kill(child_pid, SIGCONT);
                sleep(1);

            }

            act.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
            sigaction(SIGCHLD, &act, NULL);

            child_pid = fork();
            if(child_pid == 0){
                while(0 == 0);
            }
            else{
                puts("");
                puts("Flag SA_NOCLDSTOP is active");
                sleep(1);
                for (int i = 0; i < 5; i++) {
                    puts("Sending SIGSTOP to child process");
                    kill(child_pid, SIGSTOP);
                    sleep(1);
                    puts("Sending SIGCONT to child process");
                    kill(child_pid, SIGCONT);
                    sleep(1);
                }

                puts("Sending SIGKILL to child process");
                kill(child_pid, SIGKILL);
                sleep(1);
            }
            break;

        case RESETHAND:
            act.sa_sigaction = siginfo_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = SA_SIGINFO | SA_RESETHAND;
            sigaction(SIGUSR1, &act, NULL);

            for(int i = 0; i < 10; i++){
                sleep(1);
                raise(SIGUSR1);
            }
    }




}

void print_option_list(){
    puts("Correct format is: ./main option1 \n");
    puts("List of options: \n1)SIGINFO - set sa_flag to SA_SIGINFO\n2)NOCLDSTOP - set sa_flag to NOCLDSTOP\n"
         "3)RESETHAND - set sa_flag to RESETHAND\n");
}

int get_option(char * string_option){
    if (strcmp(string_option, "SIGINFO") == 0) return SIGINFO;
    if (strcmp(string_option, "NOCLDSTOP") == 0) return NOCLDSTOP;
    if (strcmp(string_option, "RESETHAND") == 0) return RESETHAND;
    return ERROR;
}

void siginfo_handler(int signo, siginfo_t *info, void *other){
    puts("Handling signal...");
    printf("Signal number: %d\n", signo);
    printf("PID: %d\n", info->si_pid);
    printf("PARENT PID %d\n", getppid());
    printf("Real user ID of sending process: %d\n", info->si_uid);
    printf("Potencial error number: %d \n", info->si_errno);
    printf("Additional info: %d\n", info->si_value.sival_int);
    puts("__________________________________________________");
}

