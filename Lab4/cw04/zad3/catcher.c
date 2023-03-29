#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "string.h"
#include "stdlib.h"
#include<time.h>

#define COUNT 1
#define TIME 2
#define REQUEST 3
#define TIME_SEC 4
#define FINISH 5

//Global variable
int request_number = 0;
int option_number = 0; // {1: count_hundred, 2: print_time, 3: print_request, 4: print_time, 5: finish}

void count_hundred();
void print_time();
void print_request(int request_number);
void finish();
void handler(int signo, siginfo_t *info, void *other);

int main(){
    printf("Catcher started \n PID: %d\n", getpid());

    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    while (1) {
//        If the task is already done
        if (option_number == TIME_SEC) {
            print_time();
            sleep(1);
        }
    }
    return 0;
}


void count_hundred(){
    printf("Catcher: Print 1..100 numbers\n");
    for(int i = 0; i < 101; i++){
        printf("%d\n", i);
    }
}

void print_time(){
    printf("Time: ");
    time_t s;
    struct tm* curr_time;
    s = time(NULL);
    curr_time = localtime(&s);
    printf("%02d:%02d:%02d\n", curr_time->tm_hour, curr_time->tm_min,
           curr_time->tm_sec);
}


void print_request(int request_number){
    printf("Number of request: %d\n", request_number);
}

void finish(){
    printf("Catcher finishes work\n");
    exit(0);
}

void handler(int signo, siginfo_t *info, void *other){
    request_number++;
    int test_option_number =  info->si_value.sival_int;
    if(test_option_number <= 0 || test_option_number > 5){
        printf("%d is incorrect command\n", test_option_number);
        printf("Confirmation sending...\n");
        kill(info->si_pid, SIGUSR1);
        return;
    }

    option_number = test_option_number;



    switch (option_number) {
        case COUNT:
            count_hundred();
            break;
        case TIME:
            print_time();
            break;
        case REQUEST:
            print_request(request_number);
            break;
        case FINISH:
            kill(info->si_pid, SIGUSR1);
            finish();
    }

    printf("Confirmation sending...\n");
    kill(info->si_pid, SIGUSR1);
}
