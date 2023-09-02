#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "constants.h"
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <mqueue.h>

int KEEP_RUNNING = 1;
char DELIM[] = " ";
int MY_ID;
mqd_t SERVER_ID;

int get_random_key();
void stop();
int send_request(char * message);
void print_correct_options();
int wait_for_input();
void get_parameters(int * sender_id, int * receiver_id, char * message, char * ptr);


int main(){

    struct mq_attr attr;
    attr.mq_msgsize = MESSAGE_LENGTH;
    attr.mq_maxmsg = MAX_CLIENT + 1;
    attr.mq_curmsgs = 0;
    printf("Open server queue\n");
    if ((SERVER_ID = mq_open(PATH, O_WRONLY, NULL)) == -1){
        perror("mq_open");
        exit(-1);
    }

    char * my_queue_name = malloc(124);
    snprintf(my_queue_name, 124, "/queue_%d", getpid());

    mqd_t my_queue_id;
    if ((my_queue_id = mq_open(my_queue_name, O_RDONLY | O_CREAT | O_NONBLOCK, 0666, &attr)) == -1) {
        perror("mq_open");
        exit(-1);
    }

    signal(SIGINT, &stop);

    char buffer[MESSAGE_LENGTH] = "";
    snprintf(buffer, MESSAGE_LENGTH, "%d %d %s", -1, -1, my_queue_name);

    if(mq_send(SERVER_ID, buffer, strlen(buffer), INIT) == -1){
        perror("mq_send");
        exit(-1);
    }


    unsigned int prior;
    int sender_id, receiver_id;

    char message[MESSAGE_LENGTH] = "";

    sleep(1);

    if(mq_receive(my_queue_id, buffer, MESSAGE_LENGTH, &prior) != -1){
        get_parameters(&sender_id, &receiver_id, message, buffer);
        if (prior == INIT) MY_ID = receiver_id;
        else{
            printf("Error while init\n");
            exit(-1);
        }
    }
    else{
        perror("mq_receive");
        exit(-1);
    }
    printf("My id is equal %d\n", MY_ID);


    char line_buffer[MESSAGE_LENGTH + 256];
    char response_buffer[MESSAGE_LENGTH];
    int ready_to_read;
    while(KEEP_RUNNING == 1){

//        SENDING SECTION
        ready_to_read = wait_for_input();
        if(ready_to_read == -1){
            perror("select");
            exit(-1);
        }
        else if(ready_to_read == 0);
        else
        {
            fgets(line_buffer, MESSAGE_LENGTH + 256, stdin);

            int error = send_request(line_buffer);

            if (error == -1) print_correct_options();
        }

        if(mq_receive(my_queue_id, response_buffer, MESSAGE_LENGTH, &prior) == -1) continue;

        char response[MESSAGE_LENGTH];
        switch (prior) {
            case STOP:
                printf("Ending working...\n");
                KEEP_RUNNING = 0;
                continue;
            default:
                get_parameters(&sender_id, &receiver_id, response, response_buffer);
                printf("Received message from client %d: %s\n", sender_id, response);
        }

    }
    printf("End working...\n");
    mq_close(SERVER_ID);
    mq_close(my_queue_id);

    sleep(1);
    mq_unlink(my_queue_name);
    free(my_queue_name);
    return 0;
}


int get_random_key(){
    srand(time(NULL));

    return ftok(PATH, rand());
}

void stop(){
    printf("Ending the process\n");

    char message_to_send[MESSAGE_LENGTH];
    snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", MY_ID, -1, "STOP");


    if(mq_send(SERVER_ID, message_to_send, strlen(message_to_send), STOP) == -1){
        perror("mq_send");
        exit(-1);
    }
    mq_close(SERVER_ID);
    KEEP_RUNNING = 0;
}


int send_request(char * message){
//    int max_options = 2;

    char *argv[MESSAGE_LENGTH];
    int counter = 0;


    char * arg = strtok(message, DELIM);
    argv[0] = arg;

    arg = strtok(NULL, DELIM);

    if(arg != NULL) {
        argv[1] = arg;
        counter++;
    }

    arg = strtok(NULL, "");
    if(arg != NULL) {
        argv[2] = arg;
        counter++;
    }

    char message_to_send[MESSAGE_LENGTH];
    unsigned int prior = -1;

    if(strcmp(argv[0], "STOP\n") == 0){
        KEEP_RUNNING = 0;
        snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", MY_ID, -1, "STOP");

        prior = STOP;
    }

    else if(strcmp(argv[0], "2ALL") == 0){
        char result[MESSAGE_LENGTH - 32];
        strcpy(result, argv[1]);
        if(counter == 2) {
            strcat(result, " ");
            strcat(result, argv[2]);
        }
        snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", MY_ID, -1, result);
        prior = TO_ALL;

    }

    else if(strcmp(argv[0], "2ONE") == 0){
        snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", MY_ID, atoi(argv[1]), argv[2]);
        prior = TO_ONE;
    }

    else if(strcmp(argv[0], "LIST\n") == 0){
        prior = LIST;
        snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", MY_ID, -1, "LIST");

    }

    if(prior == -1) return -1;

    if(mq_send(SERVER_ID, message_to_send, strlen(message_to_send), prior) == -1){
        perror("mq_send");
        exit(-1);
    }

    return 0;
}


void print_correct_options(){
    printf("Options:\n1)LIST - server on his screen list all clients\n2)2ALL - send message to all clients\n3)2ONE - "
           "send message to specified client\n4)STOP - stop working\n");
}

int wait_for_input(){
    fd_set set;
    struct timeval timeout;

    // Ustawienie timeoutu na 1 sekund
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // Czekaj na dane z wejścia przez 1 sekund
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    int ready = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

    return ready;
}


void get_parameters(int * sender_id, int * receiver_id, char * message, char * ptr){
    *sender_id = atoi(strtok(ptr, DELIM));
    *receiver_id = atoi(strtok(NULL, DELIM));
    char * arg = strtok(NULL, "\n");
    strcpy(message, arg);
}
