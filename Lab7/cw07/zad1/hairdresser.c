#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

// Constants
char * PATHNAME = "/.";
int WAITING_ID = 0, CHAIRS_ID = 1, HAIRDRESSER_ID = 2;
int ROOMS[3];
int * WR, *CR, *HR;
int HAIR_TYPES_TIME[] = {1,2,3,5,8,13};
int HAIR_NUMBER = 6;

int KEEP_RUNNING = 1;
int FPS = 5;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};



void init_semaphore(int semaphore_id, int semaphore_type, int start_value);
int * create_rooms(int capacity, int flag);
void clean(int * waiting_room, int * haircut_room, int * workers_room);
void handler(int signo);
int pick_haircut();
int decrement_semaphore(int semaphore_id, int semaphore_type, int flag);
int increment_semaphore(int semaphore_id, int semaphore_type, int flag);
int pick_chair_id(int * chairs, int client_id, int N);
int pick_hairdresser_id(int * hairdressers, int client_id, int M);

int main(int argc, char ** argv ){
    int N, M, P;
    if (argc < 4){
        printf("Error! Incorrect number of arguments\n");
        exit(-1);
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);
    P = atoi(argv[3]);

    if (M < N){
        printf("Error! M should be at least N\n");
        exit(-1);
    }

//    SEMAPHORES
    int semaphores = semget(ftok(PATHNAME, 1), 3, IPC_CREAT | 0666);

    init_semaphore(semaphores, WAITING_ID, P);
    init_semaphore(semaphores, CHAIRS_ID, N);
    init_semaphore(semaphores, HAIRDRESSER_ID, M);





//        IPC

    int * waiting_room, * chairs_room, * hairdresser_room;
    waiting_room = create_rooms(P, WAITING_ID);
    WR = waiting_room;
    chairs_room = create_rooms(N, CHAIRS_ID);
    CR = chairs_room;
    hairdresser_room = create_rooms(M, HAIRDRESSER_ID);
    HR = hairdresser_room;

//    workers_room[0] = 7;
//    printf("workers_room[0] = %d\n", workers_room[0]);

//    SIMULATION
//    PREPARE FOR ENDING
    struct sigaction act;
    act.sa_handler = &handler;

    sigaction(SIGINT, &act, NULL);

    while(KEEP_RUNNING){
        // Child is the client
        int haircut_number = pick_haircut();
        if(fork() == 0){

            if(haircut_number == HAIR_NUMBER){
                printf("[Hairdresser] Waiting chairs %d; Free Chairs %d; Free Hairdresser %d \n",
                    semctl(semaphores, WAITING_ID, GETVAL),
                    semctl(semaphores, CHAIRS_ID, GETVAL),
                    semctl(semaphores, HAIRDRESSER_ID, GETVAL));
                exit(-1);
            }
            printf("\n######################################\n");
            int time_waiting = HAIR_TYPES_TIME[haircut_number];

            pid_t my_pid = getpid();

//            Try to get haircut
            int chairs_counter = semctl(semaphores, CHAIRS_ID, GETVAL);

            if(chairs_counter == 0){

                printf("[Client %d] No space in haircut room. Try to wait in waiting room\n", my_pid);

//                Try to wait in waiting room
                int waiting_counter = semctl(semaphores, WAITING_ID, GETVAL);
                if (waiting_counter == 0){
                    printf("[Client %d] No space in waiting room. Leave the hairdresser\n", my_pid);
                    exit(-1);
                }

//                Wait in waiting room
                printf("[Client %d] Waiting in waiting room\n", my_pid);
                decrement_semaphore(semaphores, WAITING_ID, 0);
                decrement_semaphore(semaphores, CHAIRS_ID, 0);
                increment_semaphore(semaphores, WAITING_ID, 0);
                decrement_semaphore(semaphores, HAIRDRESSER_ID, 0);

            }
            else{
                decrement_semaphore(semaphores, CHAIRS_ID, 0);
                decrement_semaphore(semaphores, HAIRDRESSER_ID, 0);
            }

//            Ready to get haircut
//            Pick the chair
            int chair = pick_chair_id(chairs_room, my_pid, N);
            int hairdresser = pick_hairdresser_id(hairdresser_room, my_pid, M);
            if(chair == -1 || hairdresser == -1){
                printf("Error while picking chair and hairdresser\n");
                exit(-1);
            }

//            Getting haircut
            printf("[Client %d] Getting haircut ( waiting %d sec) \n", my_pid, time_waiting);
//            printf("Coś tam robie jeszecze\n");
            sleep(time_waiting);

            chairs_room[chair] = -1;
            hairdresser_room[hairdresser] = -1;
            increment_semaphore(semaphores, CHAIRS_ID, 0);
            increment_semaphore(semaphores, HAIRDRESSER_ID, 0);

            printf("[Client %d] Got haircut and leaved the hairdresser\n", my_pid);
            printf("[Hairdresser] Waiting chairs %d; Free Chairs %d; Free Hairdresser %d \n",
                    semctl(semaphores, WAITING_ID, GETVAL),
                    semctl(semaphores, CHAIRS_ID, GETVAL),
                    semctl(semaphores, HAIRDRESSER_ID, GETVAL));
            printf("######################################\n");
            exit(0);
        }
    }
    return 0;
}

void init_semaphore(int semaphore_id, int semaphore_type, int start_value){
    union semun arg;
    arg.val = start_value;
    semctl(semaphore_id, semaphore_type, SETVAL, arg);
}

int * create_rooms(int capacity, int flag){
    int room_id;
    room_id = shmget(ftok(PATHNAME, flag), capacity * sizeof (int), IPC_CREAT | 0666);

    ROOMS[flag] = room_id;

    int * room = malloc(sizeof (int *));
    if ((room = (int *) shmat(room_id, NULL, 0)) == (int * ) -1){
        perror("Room can not be created");
        exit(-1);
    }

    for(int i = 0; i < capacity; i++){
        room[i] = -1;
    }

    return room;
}

void clean(int * waiting_room, int * haircut_room, int * workers_room){
    shmdt(waiting_room);
    shmdt(haircut_room);
    shmdt(workers_room);

    for (int i = 0; i < 3; i++){
        shmctl(ROOMS[i], IPC_RMID, NULL);
    }

}

void handler(int signo){
    KEEP_RUNNING = 0;
    printf("\nCleaning...\n");
    clean(WR, CR, HR);
    exit(0);
}

int pick_haircut(){
//    printf("[Hairdresser] Pick the haircut(number between 0 and 5)\n");
    int haircut_number;
    scanf("%d", &haircut_number);
    if (haircut_number > HAIR_NUMBER){
        printf("[Hairdresser] There is no haircut with this id\n");
        exit(-1);
    }
    return haircut_number;
}

int decrement_semaphore(int semaphore_id, int semaphore_type, int flag){
    struct sembuf buf[1];
    buf[0].sem_num = semaphore_type;
    buf[0].sem_op = -1;
    buf[0].sem_flg = flag;

    int error = semop(semaphore_id, buf, 1);
//    printf("Error %d\n", error);
    return error;
}

int increment_semaphore(int semaphore_id, int semaphore_type, int flag){
    struct sembuf buf[1];
    buf[0].sem_num = semaphore_type;
    buf[0].sem_op = 1;
    buf[0].sem_flg = flag;

    int error = semop(semaphore_id, buf, 1);
//    printf("Error %d\n", error);
    return error;
}

int pick_chair_id(int * chairs, int client_id, int N){
    srand(time(NULL));
    for (int i = rand() % N, j = 0; j < N; i++, j++){
        i = i % N;
        if (chairs[i] == -1){
            chairs[i] = client_id;
            return i;
        }
    }
    return -1;
}

int pick_hairdresser_id(int * hairdressers, int client_id, int M){
    srand(time(NULL));
    for(int i = rand() % M, j = 0; j < M; i++, j++){
        i = i % M;
        if(hairdressers[i] == -1){
            hairdressers[i] = client_id;
            return i;
        }
    }
    return -1;
}
