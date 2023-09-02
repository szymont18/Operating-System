#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
bool KEEP_RUNNING = true;

void handler(int signo);
void basic(int signo);

void fill_args(thread_args ** all_args, char * src, char * dst, int height, int width);
void * thread_routine(void * args);


int main()
{
	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

//    Ending simulation
    struct sigaction act;
    act.sa_handler = &handler;
    sigaction(SIGINT, &act, NULL);

//  Simulation step signal
    struct sigaction simulation_step_sig;
    simulation_step_sig.sa_handler = &basic;
    simulation_step_sig.sa_flags = 0;
    sigfillset(&simulation_step_sig.sa_mask);
    sigaction(SIGUSR1, &simulation_step_sig, NULL);

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

    pthread_t * tids = calloc((CELLS_WIDTH * CELLS_HEIGHT),  sizeof (pthread_t));
    thread_args ** t_args = malloc((CELLS_WIDTH * CELLS_HEIGHT) * sizeof (thread_args *));

    init_grid(foreground);

//    To Test
//    char foreground_array[] = {0,0,0,0,0,0,0,0,0,
//            0,0,0,0,0,0,0,0,0,
//                  0,0,1,0,0,0,1,0,0,
//                  0,0,0,1,0,1,0,0,0,
//                  0,0,0,0,1,0,0,0,0,
//                  0,0,0,1,0,1,0,0,0,
//                  0,0,1,0,0,0,1,0,0,
//                  0,0,0,0,0,0,0,0,0,
//                  0,0,0,0,0,0,0,0,0};
//    memcpy(foreground, foreground_array, sizeof(foreground_array));

//    Create threads
    for (int i = 0; i < CELLS_WIDTH * CELLS_HEIGHT; i++){
        t_args[i] = malloc(sizeof (thread_args));
        t_args[i]->col = i % CELLS_WIDTH;
        t_args[i]->row = i / CELLS_WIDTH;
        t_args[i]->dst = background;
        t_args[i]->src = foreground;

        pthread_create(&(tids[i]), NULL, &thread_routine, (void *) (t_args[i]));
    }

	while (KEEP_RUNNING)
	{
		draw_grid(foreground);
		usleep(500 * 1000);
		// Step simulation

//      Update for each thread

        for(int i = 0; i < CELLS_WIDTH * CELLS_HEIGHT; i++){
            pthread_kill(tids[i], SIGUSR1);
        }
		tmp = foreground;
		foreground = background;
		background = tmp;
	}

    free(tids);
    for(int i = 0; i < CELLS_WIDTH * CELLS_HEIGHT; i++){
        free(t_args[i]);
    }
    free(t_args);
	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}


void handler(int signo){
    printf("Ending process...\n");
    KEEP_RUNNING = false;
}

void basic(int signo){
}


void * thread_routine(void * args){
    thread_args * t_args = (thread_args *) args;
    char * tmp;

    while(true){
        pause();
        t_args->dst[t_args->row * CELLS_WIDTH + t_args->col] = is_alive(t_args->row, t_args->col, t_args->src);

        tmp = t_args->src;
        t_args->src = t_args->dst;
        t_args->dst = tmp;

    }
    return NULL;

}