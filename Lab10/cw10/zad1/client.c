#include "common.h"

int IS_WEB = 0;
int server_port;
char * server_ip;
char * server_path;
int server_socket;

int keep_running = 1;
int epoll_fd;

int init_socket();
int wait_for_input();
void * keyboard_client(void * arg);
void * receive_server(void * arg);
void handler(int signo);
enum REQUEST_TYPE get_request(char * message);

int main(int argc, char ** argv){

    if (argc < 4){
        printf("Invalid number of arguments\nCorrect: ./client nickname local/web pathname/port [ip]");
        exit(-1);
    }
    if (strcmp(argv[2], "web") == 0){
        IS_WEB = 1;
    }
    if(strcmp(argv[2], "local") != 0 && IS_WEB == -1){
        printf("Invalid argument %s Should be web or local\n", argv[2]);
        exit(-1);
    }

    if (IS_WEB){
        printf("ISWEB = %d\n", IS_WEB);
        if (argc < 5){
            printf("Invalid number of arguments\nCorrect: ./client nickname local/web pathname/port [ip]\n");
            exit(-1);
        }
        server_port = atoi(argv[3]);
        server_ip = calloc(strlen(argv[4]), sizeof (char));
        strcpy(server_ip, argv[4]);

    }

    else{
        server_path = calloc(strlen(argv[3]), sizeof (char));
        strcpy(server_path, argv[3]);
    }

    struct sigaction action;
    action.sa_handler = &handler;
    sigaction(SIGINT, &action, NULL);


    char nickname[MAX_MESSAGE] = {0};
    strcpy(nickname, argv[1]);
    server_socket = init_socket();
    epoll_fd = epoll_create1(0);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = server_socket;



    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1){
        perror("Epoll server");
    }
//    Send init request - name
    printf("[Client]Sending nickname = %s\n", nickname);
    if (send(server_socket, nickname, strlen(nickname), 0) == -1){
        perror("send");
    }
//  Wait for keyboard
    pthread_t keyboard_thread;
    pthread_create(&keyboard_thread, NULL, &keyboard_client, NULL);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, &receive_server, NULL);

    sleep(1);
    pthread_join(keyboard_thread, NULL);
    pthread_join(server_thread, NULL);
    return 0;
}

int init_socket(){
    int fd = -1;
    if (IS_WEB){
        if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("Web socket");
            exit(-1);
        }
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server_port);
        addr.sin_zero[0] = '\0'; // (???)
        inet_pton(AF_INET, server_ip, &addr.sin_addr);


        if(connect(fd, (struct sockaddr *)&addr, sizeof (addr)) == -1){
            printf("Connect Web");
        }
    }
    else{
        if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
            perror("Local socket");
            exit(-1);
        }

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, server_path);

        if (connect(fd, (struct sockaddr *) &addr, sizeof (addr)) == -1){
            perror("Connect Local");
            exit(-1);
        }
    }

    return fd;
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

void * keyboard_client(void * arg){
    int ready_to_read;
    while(keep_running){
        ready_to_read = wait_for_input();

        if(ready_to_read == -1){
            perror("select");
            exit(-1);
        }
        else if(ready_to_read == 0);
        else
        {
            char send_buffer[MAX_MESSAGE] = {0};
            fgets(send_buffer, MAX_MESSAGE, stdin);

            char check_buffer[MAX_MESSAGE] = "";
            strcpy(check_buffer, send_buffer);
            if(get_request(check_buffer) == STOP) handler(0);

            if(send(server_socket, send_buffer, MAX_MESSAGE, 0) == -1){
                perror("Send");
            }
        }

    }
    return NULL;
}


enum REQUEST_TYPE get_request(char * message){
    if (strcmp(message, "STOP\n") == 0) return STOP;
    const char delim[] = " ";
    char *token = strtok(message, delim);

    if (strcmp(token, "CONNECTED") == 0) return CONNECTED;
    if (strcmp(token, "PING") == 0) return PING;
    if (strcmp(token, "STOP") == 0) return STOP;
    if (strcmp(token, "NO_SPACE") == 0) return NO_SPACE;
    if (strcmp(token, "TO_ONE") == 0) return TO_ONE;
    if (strcmp(token, "TO_ALL") == 0) return TO_ALL;
    if (strcmp(token, "LIST") == 0) return LIST;



    return NOT_INIT;
}

void * receive_server(void * arg){
    int clients_request_number = 0;
    enum REQUEST_TYPE  request;
    char buffer[MAX_MESSAGE];

    struct epoll_event all_events[MAX_CLIENT + 2];
    while(keep_running){
        clients_request_number = 0;
        if((clients_request_number = epoll_wait(epoll_fd, all_events, MAX_CLIENT + 2, 5)) == -1){
            perror("Epoll_wait");
        }

        for(int i = 0; i < clients_request_number; i++) {

            if (all_events[i].data.fd == server_socket) {
                char receive_buffer[MAX_MESSAGE] = "";
                if (recv(all_events[i].data.fd, receive_buffer, MAX_MESSAGE, 0) == -1) {
                    perror("Recv");
                }

                char receive_buffer2[MAX_MESSAGE] = "";
                strcpy(receive_buffer2, receive_buffer);
                request = get_request(receive_buffer2);
                switch (request) {
                    case CONNECTED:
                        printf("[Client] Connected to server\n");
                        break;
                    case PING:
                        sprintf(buffer, "PING");
                        send(server_socket, buffer, MAX_MESSAGE, 0);
                        printf("[Client] Receive ping from server\n[Client] Send repond\n");
                        break;
                    case STOP:
                        printf("[Client] Server send stop request\n[Client] End working...\n");
                        keep_running = 0;
                        if (IS_WEB){
                            free(server_ip);
                        }
                        else{
                            free(server_path);
                        }
                        shutdown(server_socket, SHUT_RDWR);
                        close(server_socket);
                        break;
                    case NO_SPACE:
                        printf("[Client] Server is full\n");
                        printf("[Client] End working...\n");
                        keep_running = 0;
                        break;
                    case TO_ONE:
                        printf("[Client] %s\n", receive_buffer);
                        break;
                    case TO_ALL:
                        printf("[Client] %s\n", receive_buffer);
                        break;
                    case LIST:
                        printf("[Client] %s\n", receive_buffer);
                    default:
                        printf("[Client] %s\n", receive_buffer);
                        break;
                }
            }
        }
    }
    return NULL;
}

void handler(int signo){

    keep_running = 0;
    char buffer[MAX_MESSAGE] = {0};
    sprintf(buffer, "STOP");
    if(send(server_socket, buffer, MAX_MESSAGE, 0) == -1){
        perror("send");
    }

    sleep(1);
    if (IS_WEB){
        free(server_ip);
    }
    else{
        free(server_path);
    }


    printf("[Client] End working...\n");
    sleep(1);

    exit(0);

}