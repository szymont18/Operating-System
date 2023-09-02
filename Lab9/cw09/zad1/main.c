#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>


// Constants
int REINDEER_NUMBER = 9;
int ELVES_NUMBER = 10;
int MAX_ITERATION = 3;

// Mutexes
pthread_mutex_t reinder_mtx = PTHREAD_MUTEX_INITIALIZER;
int waiting_reinders = 0;
pthread_mutex_t elves_mtx = PTHREAD_MUTEX_INITIALIZER;

int waiting_elves = 0;
pthread_t * waiting_elves_id;
int elves_come_back = 0;

// Conds
pthread_cond_t reinders_arrived   = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_arrived   = PTHREAD_COND_INITIALIZER;

// Functions
void * do_reindeer(void * arg);
void * do_elves(void * arg);

int main(){

    srand(time(NULL));


    printf("Start simulation...\n");
    pthread_mutexattr_t attr;
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&reinder_mtx, &attr);
    pthread_mutex_init(&elves_mtx, &attr);

//    Creating thread's
    pthread_t * reindeer_thread = calloc(REINDEER_NUMBER, sizeof (pthread_t));
    pthread_t * elves_thread = calloc(ELVES_NUMBER, sizeof (pthread_t));

    waiting_elves_id = calloc(3, sizeof (pthread_t));

    for (int i = 0 ; i < REINDEER_NUMBER; i++){
        pthread_create(&(reindeer_thread[i]), NULL, &do_reindeer, NULL);
    }

    for(int i = 0; i < ELVES_NUMBER; i++){
        pthread_create(&(elves_thread[i]), NULL, &do_elves, NULL);
    }

//    Santa action

    int present_delivers = 0;

    int wakes_up = 0;
    while(present_delivers < MAX_ITERATION){
//        Elves
        pthread_mutex_lock(&elves_mtx);

        if(waiting_elves == 3){
            if (wakes_up == 0) {
                printf("[Mikołaj] Budze sie\n");
                wakes_up = 1;
            }
            printf("[Mikołaj] Rozwiązuje problemy %lu, %lu, %lu elfów\n", waiting_elves_id[0], waiting_elves_id[1],
                   waiting_elves_id[2]);
            pthread_mutex_unlock(&elves_mtx);

            int working_time;
            for(int i = 0; i < 3; i++){
                working_time = rand() % 2 + 1;
                printf("[Mikołaj] Rozwiazuje problem elfa %lu\n", waiting_elves_id[i]);
                sleep(working_time);
                pthread_cond_broadcast(&elves_arrived);
            }
            pthread_mutex_lock(&elves_mtx);
            printf("[Mikołaj] Koniec problemów z elfami\n");
            waiting_elves = 0;
            pthread_mutex_unlock(&elves_mtx);
        }

        pthread_mutex_unlock(&elves_mtx);

//        Reindeers
        pthread_mutex_lock(&reinder_mtx);
        if(waiting_reinders == 9) {
            if(wakes_up == 0) {
                printf("[Mikołaj] Budze sie\n");
                wakes_up = 1;
            }
            printf("[Mikołaj] Dostarczam zabawki\n");
            int delivery_time = rand() % 3 + 2;
            sleep(delivery_time);

            waiting_reinders = 0;

            pthread_cond_broadcast(&reinders_arrived);
            present_delivers++;
        }
        pthread_mutex_unlock(&reinder_mtx);


        if (wakes_up == 1) {
            wakes_up = 0;
            printf("[Mikołaj]Zasypiam\n");
        }
    }
    printf("Ending simulation...\n");
    for(int i = 0; i < REINDEER_NUMBER; i++){
        pthread_kill(reindeer_thread[i], SIGKILL);
    }

    for(int i = 0; i < ELVES_NUMBER; i++){
        pthread_kill(elves_thread[i], SIGKILL);
    }

    free(reindeer_thread);
    free(elves_thread);
    free(waiting_elves_id);

    return 0;
}


void * do_reindeer(void * arg){
    int holiday_time;
    pthread_t my_tid = pthread_self();
    while(1){
        holiday_time = rand() % 6 + 5;

        sleep(holiday_time);

        pthread_mutex_lock(&reinder_mtx);
        waiting_reinders++;
        printf("[Renifer] Czeka %d reniferów na mikołaja. TID: %lu\n", waiting_reinders, my_tid);

        if (waiting_reinders == 9) {
            printf("[Renifer] Powiadamiam  Mikołaja. TID %lu\n", my_tid);
            pthread_cond_broadcast(&reinders_arrived);
        }
        pthread_mutex_unlock(&reinder_mtx);


        pthread_mutex_lock(&reinder_mtx);
        while(waiting_reinders > 0){
            pthread_cond_wait(&reinders_arrived, &reinder_mtx);
        }
        pthread_mutex_unlock(&reinder_mtx);

    }

    return NULL;
}


void * do_elves(void * arg){
    int working_time;
    pthread_t my_tid = pthread_self();

    while(1){
        working_time = rand() % 5 + 2;

        sleep(working_time);
        pthread_mutex_lock(&elves_mtx);

        if(waiting_elves < 3){
            int my_id = waiting_elves;
            waiting_elves_id[my_id] = my_tid;
            waiting_elves++;
            printf("[Elf] Czeka %d elfów na mikołaja. TID %lu\n", waiting_elves, my_tid);

            pthread_mutex_unlock(&elves_mtx);


            pthread_mutex_lock(&elves_mtx);
            while(waiting_elves > my_id) {
                pthread_cond_wait(&elves_arrived, &elves_mtx);
            }
            pthread_mutex_unlock(&elves_mtx);


        }

        else{
            pthread_mutex_unlock(&elves_mtx);
            working_time = rand() % 3 + 2;
            printf("[Elf] Rozwiazuje problem samemu %lu\n", my_tid);
            sleep(working_time);
        }
    }
    return NULL;


}
