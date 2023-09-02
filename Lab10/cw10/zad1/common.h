#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#include "sys/epoll.h"
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define MAX_CLIENT 11
#define MAX_MESSAGE 256
#define RESPOND_TIME 10

enum REQUEST_TYPE {
    INIT,
    NOT_INIT,
    PING, // To client
    TO_ONE,
    TO_ALL,
    LIST,
    STOP, // To client
    NO_SPACE,
    CONNECTED,
    INVALID
};

