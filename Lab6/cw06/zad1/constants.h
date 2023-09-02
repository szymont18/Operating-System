#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>


enum Message{
    INIT =  1,
    LIST =  2,
    TO_ALL = 3,
    TO_ONE = 4,
    STOP = 5,
    ERROR = 6,
    DEFAULT = -1
}Message;


#define MESSAGE_LENGTH 256
#define MAX_CLIENT 5

#define PATH getenv("HOME")
#define MAGIC_NUMBER 181
#define SERVER_KEY ftok(PATH, MAGIC_NUMBER)


struct Mymsgbuf{
    long mtype;
    char mtext[MESSAGE_LENGTH];

    // Additional variables
    int sender_id;
    int receiver_id;
    int queue_id;
    char * data;
}Mymsgbuf;


