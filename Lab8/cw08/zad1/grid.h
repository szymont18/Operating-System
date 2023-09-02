#pragma once
#include <stdbool.h>

char *create_grid();
void destroy_grid(char *grid);
void draw_grid(char *grid);
void init_grid(char *grid);
bool is_alive(int row, int col, char *grid);
void update_grid(char *src, char *dst);

#define CELLS_WIDTH 10
#define CELLS_HEIGHT 10

typedef struct thread_args{
    int col;
    int row;
    char * src;
    char * dst;

}thread_args;