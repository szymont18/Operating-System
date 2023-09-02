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
int min(int a, int b);


int main(int argc, char ** argv){
    if(argc != 2){
        printf("Error! Correct number of arguments is one\n");
        exit(-1);
    }

    int thread_number = atoi(argv[1]);

    if(thread_number <= 0){
        printf("Number of thread must be positive number\n");
        exit(-1);
    }


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

    pthread_t * tids = calloc(thread_number,  sizeof (pthread_t));
    thread_args ** t_args = malloc(thread_number * sizeof (thread_args *));

    init_grid(foreground);

//    To Test
    /**
     char foreground_array[] = {0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,
                                0,0,1,0,0,0,1,0,0,
                                0,0,0,1,0,1,0,0,0,
                                0,0,0,0,1,0,0,0,0,
                                0,0,0,1,0,1,0,0,0,
                                0,0,1,0,0,0,1,0,0,
                                0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0};

    memcpy(foreground, foreground_array, sizeof(foreground_array));
    **/

//    Create threads
    int number_of_cells = CELLS_HEIGHT * CELLS_WIDTH;
    thread_number = min(thread_number, number_of_cells);
    int cells_per_thread = number_of_cells / thread_number;
    int start_cel = 0;

    for (int i = 0; i < thread_number; i++){
        t_args[i] = malloc(sizeof (thread_args));
        t_args[i]->start_index = start_cel;
        t_args[i]->end_index = (i == thread_number - 1) ? number_of_cells : min(start_cel + cells_per_thread, number_of_cells);
        t_args[i]->dst = background;
        t_args[i]->src = foreground;


        pthread_create(&(tids[i]), NULL, &thread_routine, (void *) (t_args[i]));

        start_cel = t_args[i]->end_index;
    }
	while (KEEP_RUNNING)
	{
		draw_grid(foreground);
		usleep(500 * 1000);
		// Step simulation
//      Update for each thread
        for(int i = 0; i < thread_number; i++){
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
        for (int i = t_args->start_index; i < t_args->end_index; i++) {
            t_args->dst[i] = is_alive(i / CELLS_WIDTH, i % CELLS_WIDTH, t_args->src);
        }
        tmp = t_args->src;
        t_args->src = t_args->dst;
        t_args->dst = tmp;

    }
    return NULL;

}

int min(int a, int b) {
    return a < b ? a : b;
}
