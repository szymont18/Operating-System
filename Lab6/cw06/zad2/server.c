#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "constants.h"
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>



mqd_t clients[MAX_CLIENT];
int KEEP_RUNNING = 1;
char DELIM[] = " ";

void stop(mqd_t * clients, int client_id);
void list(mqd_t * clients);
void to_all(mqd_t * clients, int sender_id, char * message);
void to_one(mqd_t * clients, int receiver_id, int sender_id, char * message);
void save_logs(FILE * file, enum Message message_type, int client_id);
void init(mqd_t * clients, char * queue_name);
void stop_all_process(int signo);
void get_parameters(int * sender_id, int * receiver_id, char * message, char * ptr);


int main(int argc, char ** argv){

//  Create server
    mqd_t server_queue_id;
    struct mq_attr attr;
    attr.mq_msgsize = MESSAGE_LENGTH;
    attr.mq_maxmsg = MAX_CLIENT + 1;
    attr.mq_curmsgs = 0;
    printf("Start working on directory %s\n", PATH);
    if((server_queue_id = mq_open(PATH, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr)) == -1){
        perror("mq_open");
        exit(-1);
    }
    FILE * logs = fopen("logs.txt", "w");
    if(logs == NULL){
        printf("Can not open log file\n");
        exit(-1);
    }

    memset(clients, -1, sizeof(clients)); // Set all clients to -1

    signal(SIGINT, &stop_all_process);

    char  buffer[MESSAGE_LENGTH + 1];
    unsigned int  priop;
    int sender_id, receiver_id;
    char message[MESSAGE_LENGTH + 1]; // Maybe without malloc

    printf("Waiting for message...\n");

    while (KEEP_RUNNING){
//        Priorities (most important) STOP > LIST > INIT > TO_ALL > TO_ONE (least important)
        if(mq_receive(server_queue_id, buffer, MESSAGE_LENGTH, &priop) == -1) continue;

        get_parameters(&sender_id, &receiver_id, message, buffer);

        switch (priop) {
            case STOP:
                printf("STOPING\n");
                stop(clients, sender_id);
                save_logs(logs, STOP, sender_id);
                break;
            case LIST:
                list(clients);
                save_logs(logs, LIST, sender_id);
                break;
            case INIT:
                init(clients, message);
                message[strlen(message) - 1] = '\0';
                save_logs(logs, INIT, sender_id);
                break;
            case TO_ONE:
                to_one(clients, receiver_id, sender_id, message);
                save_logs(logs, TO_ONE, sender_id);
                break;
            case TO_ALL:
                to_all(clients, sender_id, message);
                save_logs(logs, TO_ALL, sender_id);
                break;
            default:break;
        }

    }
    fclose(logs);
    printf("Closing the server...\n");
    if (mq_close(server_queue_id)) { // Zamknięcie kolejki
        perror("mq_close error");
        exit(-1);
    }
    printf("Unlinking the server...\n");
    if(mq_unlink(PATH) == -1){
        perror("unlink");
        exit(-1);
    }
    return 0;
}


void stop(mqd_t * clients, int client_id){

    if (clients[client_id] == -1){
        printf("[SERVER] This client is already deleted\n");
        return;
    }
    printf("[SERVER] Delete client %d\n", client_id);
    if(mq_close(clients[client_id]) == -1){
        perror("mq_close");
        exit(-1);
    }
    clients[client_id] = -1;
}

void list(mqd_t * clients){
    printf("Listing clients\n");
    int client_number = 1;
    for(int i = 0; i < MAX_CLIENT; i++){
        if(clients[i] != -1){
            printf("Client number %d with ID %d\n", client_number, i);
            client_number++;
        }
    }
}

void to_all(mqd_t * clients, int sender_id, char * message){

    printf("Sending message '%s' to all clients\n", message);

    for(int i = 0; i < MAX_CLIENT; i++){
        if (clients[i] != -1 && i != sender_id){
            char message_to_send[MESSAGE_LENGTH];
            snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", sender_id, i, message);

            if(mq_send(clients[i], message_to_send, strlen(message_to_send), TO_ALL) == -1){
                perror("msgsnd");
                exit(-1);
            }
        }
    }
}


void to_one(mqd_t * clients, int receiver_id, int sender_id, char * message){

    printf("Sending message from %d to %d\n", sender_id, receiver_id);

    if (clients[sender_id] == -1 || clients[receiver_id] == -1){
        printf("Can not send this message (sender or receiver does not exists\n");
        exit(-1);
    }

    char message_to_send[MESSAGE_LENGTH];
    snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", sender_id, receiver_id, message);

    if(mq_send(clients[receiver_id], message_to_send, strlen(message_to_send), TO_ONE) == -1){
        perror("mq_snd");
        exit(-1);
    }
}


void init(mqd_t * clients, char * queue_name){
    int new_client_id = -1;
    for(int i = 0; i < MAX_CLIENT; i++){
        if (clients[i] == -1){
            mqd_t queue_id = mq_open(queue_name, O_WRONLY, NULL);
            if(queue_id == -1){
                perror("mq_open");
                exit(-1);
            }

            clients[i] = queue_id;
            new_client_id = i;
            break;
        }
    }
    if(new_client_id == -1){
        printf("Not enough space to add new client\n");
    }
    else {
        printf("Add new client with id %d\n", new_client_id);

        char message[MESSAGE_LENGTH] = "";
        snprintf(message, MESSAGE_LENGTH, "%d %d %s", -1, new_client_id, "INIT");

        if (mq_send(clients[new_client_id], message, strlen(message), INIT) == -1) {
            perror("mq_snd");
            exit(-1);
        }
    }
}


void stop_all_process(int signo){
    printf("Stopping all process\n");
    for (int i = 0; i < MAX_CLIENT; i++){
        if(clients[i] == -1) continue;


        char message_to_send[MESSAGE_LENGTH];
        snprintf(message_to_send, MESSAGE_LENGTH, "%d %d %s", -1, -1, "STOP");

        if(mq_send(clients[i], message_to_send, strlen(message_to_send), STOP) == -1){
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
        case INIT: request = "INIT";    break;
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
    snprintf(buffer, MESSAGE_LENGTH + 256, "RECEIVE TIME: %02d-%02d-%04d %02d:%02d:%02d.\t CLIENT ID: %d\t REQUEST %s\n",
             local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
             local->tm_hour, local->tm_min, local->tm_sec,
             client_id, request);

    fwrite(buffer, sizeof (char), strlen(buffer), file);
}

void get_parameters(int * sender_id, int * receiver_id, char * message, char * ptr){
    *sender_id = atoi(strtok(ptr, DELIM));
    *receiver_id = atoi(strtok(NULL, DELIM));
    char * arg = strtok(NULL, "\n");
    strcpy(message, arg);
}


