#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "constants.h"
#include <string.h>
#include <signal.h>
#include <time.h>

int clients[MAX_CLIENT];
int KEEP_RUNNING = 1;

void stop(int * clients, int client_id);
void list(int * clients);
void to_all(int * clients, int sender_id, char * message);
void to_one(int * clients, int receiver_id, int sender_id, char * message);
void save_logs(FILE * file, enum Message message_type, int client_id);
void init(int * clients, int queue_id);
void stop_all_process(int signo);

int main(int argc, char ** argv){

//  Create server
    int server_queue_id;
    if ((server_queue_id = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1){
        perror("msgget");
        exit(-1);
    }


    FILE * logs = fopen("logs.txt", "w");
    if(logs == NULL){
        printf("Can not open log file\n");
        exit(-1);
    }

    memset(clients, -1, sizeof(clients)); // Set all clients to -1

    struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));

    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

    signal(SIGINT, &stop_all_process);


    while (KEEP_RUNNING){
//        Priorities (most important) STOP > LIST > INIT > TO_ALL > TO_ONE (least important)
        if(msgrcv(server_queue_id, buf, buf_without_long, STOP, IPC_NOWAIT) != -1){
            stop(clients, buf->sender_id);
            save_logs(logs, buf->mtype, buf->sender_id);
        }

        if(msgrcv(server_queue_id, buf, buf_without_long, LIST, IPC_NOWAIT) != -1){
            list(clients);
            save_logs(logs, buf->mtype, buf->sender_id);
        }

        if(msgrcv(server_queue_id, buf, buf_without_long, INIT, IPC_NOWAIT) != -1){
            init(clients, buf->queue_id);
            save_logs(logs, buf->mtype, buf->sender_id);
        }

        if(msgrcv(server_queue_id, buf, buf_without_long, TO_ALL, IPC_NOWAIT) != -1){
            to_all(clients, buf->sender_id, buf->mtext);
            save_logs(logs, buf->mtype, buf->sender_id);

        }

        if(msgrcv(server_queue_id, buf, buf_without_long, TO_ONE, IPC_NOWAIT) != -1){
            to_one(clients, buf->receiver_id, buf->sender_id, buf->mtext);
            save_logs(logs, buf->mtype, buf->sender_id);
        }
    }

    free(buf);
    fclose(logs);
    if (msgctl(server_queue_id, IPC_RMID, NULL) == -1) { // usunięcie kolejki komunikatów
        perror("msgctl error");
        exit(EXIT_FAILURE);
    }
}


void stop(int * clients, int client_id){

    if (clients[client_id] == -1){
        printf("[SERVER] This client is already deleted\n");
    }
    printf("[SERVER] Delete client %d\n", client_id);
    clients[client_id] = -1;
}

void list(int * clients){
    printf("Listing clients\n");
    int client_number = 1;
    for(int i = 0; i < MAX_CLIENT; i++){
        if(clients[i] != -1){
            printf("Client number %d with ID %d\n", client_number, i);
            client_number++;
        }
    }
}

void to_all(int * clients, int sender_id, char * message){

    printf("Sending message '%s' to all clients\n", message);

    struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));
    //    buf->mtext = message; // Does not work

    strcpy(buf->mtext, message);
    buf->mtype = TO_ALL;
    buf->sender_id = sender_id;


    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

    for(int i = 0; i < MAX_CLIENT; i++){
        if (clients[i] != -1 && i != sender_id){


            if(msgsnd(clients[i], buf, buf_without_long, 0) == -1){
                perror("msgsnd");
                exit(-1);
            }
        }
    }
    free(buf);
}


void to_one(int * clients, int receiver_id, int sender_id, char * message){

    printf("Sending message from %d to %d\n", sender_id, receiver_id);

    if (clients[sender_id] == -1 || clients[receiver_id] == -1){
        printf("Can not send this message (sender or receiver does not exists\n");
        exit(-1);
    }
    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

    struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));
//    buf->mtext = message; // Does not work

    strcpy(buf->mtext, message);
    buf->mtype = TO_ONE;
    buf->sender_id = sender_id;

    if(msgsnd(clients[receiver_id], buf, buf_without_long, 0) == -1){
        perror("msgsnd");
        exit(-1);
    }
    free(buf);
}


void init(int * clients, int queue_id){
    int new_client_id = -1;

    for(int i = 0; i < MAX_CLIENT; i++){
        if (clients[i] == -1){
            clients[i] = queue_id;
            new_client_id = i;
            break;
        }
    }

    size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

    struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));
    buf->mtype = INIT;
    buf->receiver_id = new_client_id;

    if(new_client_id == -1){
        printf("Not enough space to add new client\n");
        buf->mtype = ERROR;
    }
    else printf("Add new client with id %d\n", buf->receiver_id);


    if(msgsnd(queue_id, buf, buf_without_long, 0) == -1){
        perror("msgsnd");
        exit(-1);
    }
}


void stop_all_process(int signo){
    printf("Stopping all process\n");
    for (int i = 0; i < MAX_CLIENT; i++){
        if(clients[i] == -1) continue;

        struct Mymsgbuf * buf = malloc(sizeof (Mymsgbuf));
        buf->mtype = STOP;

        size_t buf_without_long = sizeof (Mymsgbuf) - sizeof (long);

        if(msgsnd(clients[i], buf, buf_without_long, 0) == -1){
            perror("msgsnd");
            exit(-1);
        }

        stop(clients, i);
    }
    KEEP_RUNNING = 0;
}


void save_logs(FILE * file, enum Message message_type, int client_id){

    char * request;

    switch (message_type) {
        case INIT: request = "Init";    break;
        case LIST: request = "LIST";    break;
        case TO_ALL: request = "TO ALL";break;
        case TO_ONE: request = "TO ONE";break;
        case STOP:request = "STOP";     break;
        default:request = "INCORRECT";
    }
    time_t now;
    struct tm *local;
    time(&now);
    local = localtime(&now);


    char  buffer[MESSAGE_LENGTH + 256];
    snprintf(buffer, MESSAGE_LENGTH + 256, "RECEIVE TIME: %02d-%02d-%04d %02d:%02d:%02d.\t CLIENT ID: %d. \t REQUEST %s\n",
             local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
             local->tm_hour, local->tm_min, local->tm_sec,
             client_id, request);

    fwrite(buffer, sizeof (char), strlen(buffer), file);
}


