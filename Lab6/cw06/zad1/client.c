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

int KEEP_RUNNING = 1;
char DELIM[] = " ";
int MY_ID;
int SERVER_ID;

int get_random_key();
void stop();
int get_option(struct Mymsgbuf * buf, char * message);
void print_correct_options();
int wait_for_input();

int main(){

    if ((SERVER_ID = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1){
        perror("msgget");
        exit(-1);
    }

    int my_queue_id;

    if ((my_queue_id = msgget(get_random_key(), IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(-1);
    }

    signal(SIGINT, &stop);

    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);


    struct Mymsgbuf * first_buf = malloc(sizeof (Mymsgbuf));
    first_buf->mtype = INIT;
    first_buf->queue_id = my_queue_id;

    if(msgsnd(SERVER_ID, first_buf, buf_without_long, 0) == -1){
        perror("msgsnd");
        exit(-1);
    }


    if(msgrcv(my_queue_id, first_buf, buf_without_long, 0, 0) != -1){
        if(first_buf->mtype == INIT) MY_ID = first_buf->receiver_id;
        else if(first_buf->mtype == ERROR){
            printf("Error while init\n");
            exit(-1);
        }
    }
    free(first_buf);
    printf("My id is equal %d\n", MY_ID);


    struct Mymsgbuf * receive_buf = malloc(sizeof (Mymsgbuf));
    struct Mymsgbuf * sending_buf = malloc(sizeof (Mymsgbuf));

    char line_buffer[MESSAGE_LENGTH + 256];
    int ready_to_read;
    while(KEEP_RUNNING == 1){


//        RECEIVE SECTION
        if(msgrcv(my_queue_id, receive_buf, buf_without_long, STOP, IPC_NOWAIT) != -1){
            printf("Server stopped.  Ending working...\n");
            KEEP_RUNNING = 0;
            continue;
        }

        if(msgrcv(my_queue_id, receive_buf, buf_without_long, 0, IPC_NOWAIT) != -1){
            printf("Received message from client %d: %s\n", receive_buf->sender_id, receive_buf->mtext);
        }

//        SENDING SECTION
        ready_to_read = wait_for_input();
        if(ready_to_read == -1){
            perror("select");
            exit(-1);
        }
        else if(ready_to_read == 0) continue;

        fgets(line_buffer, MESSAGE_LENGTH + 256, stdin);

        int error = get_option(sending_buf, line_buffer);

        if (error == -1) continue;

        if(msgsnd(SERVER_ID, sending_buf, buf_without_long, 0) == -1){
            perror("msgsnd");
            exit(-1);
        }
    }

    return 0;
}


int get_random_key(){
    srand(time(NULL));

    return ftok(PATH, rand());
}

void stop(){
    printf("Ending the process\n");

    struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));
    buf->mtype = STOP;
    buf->sender_id = MY_ID;

    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

    if(msgsnd(SERVER_ID, buf, buf_without_long, 0) == -1){
        perror("msgsnd");
        exit(-1);
    }
    exit(0);
}


int get_option(struct Mymsgbuf * buf, char * message){
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
    if (arg != NULL) {
        argv[2] = arg;
        counter++;
    }


    buf->mtype = DEFAULT;

    if(strcmp(argv[0], "STOP\n") == 0){
        buf->mtype = STOP;
        buf->sender_id = MY_ID;
        KEEP_RUNNING = 0;
    }

    else if(strcmp(argv[0], "2ALL") == 0){
        buf->mtype = TO_ALL;
        buf->sender_id = MY_ID;
        char result[MESSAGE_LENGTH];
        strcpy(result, argv[1]);
//        printf("BEFORE STRCAT %d %s %s %s\n", counter, argv[0], argv[1], argv[2]);
        if(counter == 2) {
            strcat(result, " ");
            strcat(result, argv[2]);
        }
        strcpy(buf->mtext, result);
    }

    else if(strcmp(argv[0], "2ONE") == 0){
        buf->mtype = TO_ONE;
        buf->sender_id = MY_ID;
        buf->receiver_id = atoi(argv[1]);

        strcpy(buf->mtext, argv[2]);
    }

    else if(strcmp(argv[0], "LIST\n") == 0){
        buf->mtype = LIST;
        buf->sender_id = MY_ID;
    }

    if (buf->mtype == DEFAULT) {
        print_correct_options();
        return -1;
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


