#include "common.h"
#include <stdlib.h>
#include <stdio.h>


char * path;
int port;
int web_socket;
int local_socket;
int epoll_server_fd;
FILE * log_file;

struct epoll_event events[MAX_CLIENT];
int all_fd[MAX_CLIENT]; // -1 no_client
char * clients[MAX_CLIENT];
int ping_active[MAX_CLIENT]; // -1 not active

int keep_running = 1;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

int init_web_socket();
int init_local_socket();
void handler(int signo);
enum REQUEST_TYPE get_request(char * message);
int add_client(char * nickname, int fd);
void ping_active_check(int fd);
void send_to_one(char * buffer, int sender);
void send_to_all(char * buffer, int sender_id);
void list_clients(int sender_id);
void remove_client(int client_fd);
void * ping(void * arg);
char* getCurrentTime();
void save_logs(char * log);

int main(int argc, char ** argv){
    if (argc != 3){
        printf("Invalid number of arguments\n Correct: ./server port path");
        exit(-1);
    }

    for (int i = 0; i < MAX_CLIENT; i++){
        all_fd[i] = -1;
        clients[i] = calloc(MAX_MESSAGE, sizeof (char));
        ping_active[i] = -1;
    }

//    Kill signal
    struct sigaction action;
    action.sa_handler = &handler;
    sigaction(SIGINT, &action, NULL);


    int * indexes = malloc(sizeof (int) * (MAX_CLIENT + 2));
    for (int i = 0; i < MAX_CLIENT + 2; i++){
        indexes[i] = i;
    }


    port = atoi(argv[1]);
    path = calloc(sizeof (char), MAX_MESSAGE);
    strcpy(path, argv[2]);

    web_socket = init_web_socket();
    local_socket = init_local_socket();

    printf("Web socket %d\nLocal Socket %d\n", web_socket, local_socket);

//  Epoll_fd
    epoll_server_fd = epoll_create1(0);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    union epoll_data data;
    data.fd = web_socket;
    event.data = data;

    if (epoll_ctl(epoll_server_fd, EPOLL_CTL_ADD, web_socket, &event) == -1){
        perror("Epoll web");
    }

    struct epoll_event event2;
    event2.events = EPOLLIN | EPOLLPRI;
    union epoll_data data2;
    data2.fd = local_socket;
    event2.data = data2;

    if (epoll_ctl(epoll_server_fd, EPOLL_CTL_ADD, local_socket, &event2) == -1){
        perror("Epoll local");
    }

    log_file = fopen("log.txt", "a+");

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, &ping, NULL);

    struct epoll_event all_events[MAX_CLIENT + 2];
    int clients_request_number = 0;
    enum REQUEST_TYPE request_type;

    while(keep_running){
        clients_request_number = 0;
        if((clients_request_number = epoll_wait(epoll_server_fd, all_events, MAX_CLIENT + 2, 5)) == -1){
            perror("Epoll_wait");
//            exit(-1);
        }
        for(int i = 0; i < clients_request_number; i++){
            char * buffer = calloc(MAX_MESSAGE, sizeof (char));

            if (all_events[i].data.fd == web_socket || all_events[i].data.fd == local_socket){
                int new_client_fd;

                if((new_client_fd = accept(all_events[i].data.fd, NULL, NULL)) == -1){
                    perror("accept error");
                    free(buffer);
                    continue;
                }
                else {
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = new_client_fd;

                    if(epoll_ctl(epoll_server_fd, EPOLL_CTL_ADD, new_client_fd, &event) == -1){
                        perror("epoll_ctl");
                    }

                    if (recv(new_client_fd, buffer, MAX_MESSAGE, 0) == -1) {
                        perror("Read");
                    }
                    if(add_client(buffer, new_client_fd) == -1) {
                        free(buffer);
                        continue;
                    }
                }
            }
            if(recv(all_events[i].data.fd, (char *) buffer, MAX_MESSAGE, 0) == -1){
                free(buffer);
                continue;
            }

            char buffer2[MAX_MESSAGE] = "";
            strcpy(buffer2, buffer);

            if(strlen(buffer2) > 0) {
                request_type = get_request(buffer2);
                switch (request_type) {
                    case PING:
                        ping_active_check(all_events[i].data.fd);
                        break;
                    case TO_ONE:
                        send_to_one(buffer, all_events[i].data.fd);
                        break;
                    case TO_ALL:
                        send_to_all(buffer, all_events[i].data.fd);
                        break;
                    case LIST:
                        list_clients(all_events[i].data.fd);
                        break;
                    case STOP:
                        remove_client(all_events[i].data.fd);
                        break;
                    default:
                        break;

                }
            }
            free(buffer);
        }

    }

    return 0;
}

int init_web_socket(){

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Web socket");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_zero[0] = '\0'; // (???)

    if((bind(fd, (struct sockaddr*)&addr, sizeof (struct sockaddr))) == -1){
        perror("Web bind");
        exit(-1);
    }

    if (listen(fd, MAX_CLIENT) == -1){
        perror("Web listen");
        exit(-1);
    }

    return fd;
}

int init_local_socket(){
    int fd = -1;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Local socket");
        exit(-1);
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if((bind(fd, (struct sockaddr*)&addr, sizeof (struct sockaddr))) == -1){
        perror("Local bind");
        exit(-1);
    }

    if(listen(fd, MAX_CLIENT) == -1){
        perror("Listen");
    }

    return fd;
}

void handler(int signo){
    keep_running = 0;
    char stop[MAX_MESSAGE] = "STOP\n";
    pthread_mutex_lock(&client_mutex);
    for(int i = 0; i < MAX_CLIENT; i++){
        if(all_fd[i] != -1 && ping_active[i] != -1){

            if(send(all_fd[i], stop, MAX_MESSAGE, 0) == -1){
                perror("send stop");
            }
            else{
                printf("[Server] Send stop to %s\n", clients[i]);
            }
        }
    }

    sleep(2);
    shutdown(web_socket, SHUT_RDWR);
    close(web_socket);

    shutdown(local_socket, SHUT_RDWR);
    close(local_socket);
    pthread_mutex_unlock(&client_mutex);
    pthread_mutex_destroy(&client_mutex);
    pthread_mutex_destroy(&file_mutex);
    fclose(log_file);
    exit(0);
}

enum REQUEST_TYPE get_request(char * message){
    if (strcmp(message, "LIST\n") == 0) return LIST;
    if (strcmp(message, "STOP\n") == 0) return STOP;

    const char delim[] = " ";
    char *token = strtok(message, delim);


    if (strcmp(token, "INIT") == 0) return INIT;
    if (strcmp(token, "PING") == 0) return PING;
    if(strcmp(token, "TO_ONE") == 0) return TO_ONE;
    if (strcmp(token, "TO_ALL") == 0) return TO_ALL;

    if (strcmp(token, "STOP") == 0) return STOP;

    return INVALID;
}

int add_client(char * nickname, int fd){
    int ind = -1;
    char log[MAX_MESSAGE];

    pthread_mutex_lock(&client_mutex);
    printf("[Server] %s is trying to log in with %d fd\n", nickname, fd);

    sprintf(log, "[Server] %s is trying to log in with %d fd\n", nickname, fd);
    save_logs(log);

    for (int i = 0; i < MAX_CLIENT; i++){
        if (all_fd[i] == -1){
            ind = i;
            break;
        }
    }
    if (ind == -1){
        char buffer[MAX_MESSAGE] = "";
        sprintf(buffer, "NO_SPACE");

        if(send(fd, buffer, MAX_MESSAGE, 0) == -1){
            perror("send");
        }
        printf("[Server] Not enough space for new client\n");
        sprintf(log, "[Server] Not enough space for new client\n");
        save_logs(log);
        pthread_mutex_unlock(&client_mutex);

        return -1;
    }

    else{
        strcpy(clients[ind], nickname);
        all_fd[ind] = fd;
        ping_active[ind] = 1;

        char buffer[MAX_MESSAGE] = "";
        sprintf(buffer, "CONNECTED");
        if(send(fd, buffer, MAX_MESSAGE, 0) == -1){
            perror("send");
        }

        printf("[Server] %s log in\n", nickname);
        sprintf(log, "[Server] %s log in\n", nickname);
        save_logs(log);

    }
    pthread_mutex_unlock(&client_mutex);
    return 0;
}

void ping_active_check(int fd){
    pthread_mutex_lock(&client_mutex);
    char log[MAX_MESSAGE];
    for (int i = 0; i < MAX_CLIENT; i++){
        if (all_fd[i] == fd){
            ping_active[i] = 1; // Active
            sprintf(log, "[Server] Received ping respond from %s\n", clients[i]);
            save_logs(log);
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void send_to_one(char * buffer, int sender){
    const char delim[] = " ";
    char log[MAX_MESSAGE];
    char * receiver = strtok(buffer, delim);
    receiver = strtok(NULL, delim);

    char * message = strtok(NULL, "\n");

    pthread_mutex_lock(&client_mutex);
    printf("[Server] Sending message '%s' to %s\n", message, receiver);
    sprintf(log, "[Server] Sending message '%s' to %s\n", message, receiver);
    save_logs(log);

    int ind = -1;
    int sender_ind = -1;
    for(int i = 0; i < MAX_CLIENT; i++){
        if (ping_active[i] == 1 && strcmp(receiver, clients[i]) == 0){
            ind = i;
        }

        if (all_fd[i] == sender){
            sender_ind = i;
        }

        if (sender_ind != -1 && ind != -1) break;
    }
    if(ind == -1 || sender_ind == -1){
        printf("[Server] Can not send message to %s\n", receiver);
        sprintf(log, "[Server] Can not send message to %s\n", receiver);
        save_logs(log);

    }
    char * msg_to_send = calloc(MAX_MESSAGE, sizeof (char));
    sprintf(msg_to_send, "[%s] >> %s", clients[sender_ind], message);
    if(send(all_fd[ind], msg_to_send, MAX_MESSAGE, 0) == -1){
        perror("send");
    }

    pthread_mutex_unlock(&client_mutex);

    free(msg_to_send);
}

void send_to_all(char * buffer, int sender_id){
    const char delim[] = " ";
    char log[MAX_MESSAGE];
    char * message = strtok(buffer, delim);
    message = strtok(NULL, "\n");

    pthread_mutex_lock(&client_mutex);
    int sender;
    for(int i = 0; i < MAX_CLIENT; i++){
        if(sender_id == all_fd[i]){
            sender = i;
            break;
        }
    }

    char * msg_to_send = calloc(MAX_MESSAGE, sizeof (char));
    sprintf(msg_to_send, "[%s] >> %s", clients[sender], message);
    for(int i = 0; i < MAX_CLIENT; i++){
        if (all_fd[i] != -1 && ping_active[i] != -1){
            printf("[Server] Sending message %s to %s\n", msg_to_send, clients[i]);

            sprintf(log,"[Server] Sending message %s to %s\n", msg_to_send, clients[i]);
            save_logs(log);

            if(send(all_fd[i], msg_to_send, MAX_MESSAGE, 0) == -1){
                perror("send to all");
            }

        }
    }
    free(msg_to_send);
    pthread_mutex_unlock(&client_mutex);

}

void list_clients(int sender_id){

    pthread_mutex_lock(&client_mutex);

    char log[MAX_MESSAGE] = "[Server] List clients\n";
    sprintf(log,"[Server] List clients\n");
    save_logs(log);

    char * list_of_clients = calloc(MAX_MESSAGE, sizeof (char));

    for (int i = 0; i < MAX_CLIENT; i++){
        if(all_fd[i] != -1 && ping_active[i] != -1){
            strcat(list_of_clients, clients[i]);
            strcat(list_of_clients, " ");
        }
    }
    if(send(sender_id, list_of_clients, MAX_MESSAGE, 0) == -1){
        printf("List_clients");
    }

    free(list_of_clients);
    pthread_mutex_unlock(&client_mutex);
}

void remove_client(int client_fd){
    char log[MAX_MESSAGE];
    pthread_mutex_lock(&client_mutex);

    epoll_ctl(epoll_server_fd, EPOLL_CTL_DEL, client_fd, NULL);

    for(int i = 0; i < MAX_CLIENT; i++){
        if (all_fd[i] == client_fd){
            all_fd[i] = -1;
            ping_active[i] = -1;
            printf("[Server] Removed client %s\n", clients[i]);
            sprintf(log,"[Server] Removed client %s\n", clients[i]);
            save_logs(log);

            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}


void * ping(void * arg){

    char ping[MAX_MESSAGE] = "PING";
    char log[MAX_MESSAGE];
    while(keep_running){

        for(int i = 0; i < MAX_CLIENT; i++){
            pthread_mutex_lock(&client_mutex);
            if(all_fd[i] != -1 && ping_active[i] != -1){

                sprintf(log, "[Server] Ping %s\n", clients[i]);
                save_logs(log);
                if(send(all_fd[i], ping, MAX_MESSAGE, 0) == -1){
                    perror("send ping");
                }
                else{
                    ping_active[i] = -1;
                }
                pthread_mutex_unlock(&client_mutex);

                sleep(RESPOND_TIME);

                if(ping_active[i] == -1){
                    remove_client(all_fd[i]);
                }

                pthread_mutex_lock(&client_mutex);
            }
            pthread_mutex_unlock(&client_mutex);
        }
    }
    return NULL;
}



char* getCurrentTime() {
    time_t rawTime;
    struct tm *timeInfo;
    static char timeString[50];

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    sprintf(timeString, "%04d:%02d:%02d::%02d:%02d:%02d --- ",
            timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday,
            timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);

    return timeString;
}


void save_logs(char * log){

    char * time = getCurrentTime();

    pthread_mutex_lock(&file_mutex);
    fwrite(time, sizeof (char), strlen(time), log_file);
    fwrite(log, sizeof (char), strlen(log), log_file);
    fwrite("\n", sizeof (char), strlen("\n"), log_file);

    pthread_mutex_unlock(&file_mutex);

}