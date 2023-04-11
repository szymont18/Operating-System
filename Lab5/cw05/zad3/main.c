#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

// Integrals Boundaries
double A = 0.0;
double B = 1.0;
char * FIFO_PATH = "/tmp/integral_tmp";
int SIZE = PIPE_BUF; //from limits.h


// Functions declarations
void print_correct_format();
int get_int_number(char * string_number);
double get_double_number(char * string_number);
double min(double a, double b);

int main(int argc, char ** argv){
    if (argc < 3){
        printf("Error: Missed %d arguments\n", 3 - argc);
        print_correct_format();
        return -1;
    }

    double h = get_double_number(argv[1]);
    int n = get_int_number(argv[2]);

    if (n < 0 || h < 0){
        printf("Rectange length and number of process must be positive\n");
        return -1;
    }
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    mkfifo(FIFO_PATH, 0666);

    double left = A, step = (B - A) / n;
    pid_t child;

    for(int i = 0; i < n; i++){
        child = fork();
        if(child == 0) {
            char arg1[64], arg2[64], arg3[64];
            snprintf(arg1, 64, "%.15f", left);
            snprintf(arg2, 64, "%.15f", min(left + step, B));
            snprintf(arg3, 64, "%.15f", h);

            execl("./integral", "integral", arg1, arg2, arg3, FIFO_PATH, NULL);
            exit(0);
        }
        left += step;
    }

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1){
        perror("");
        printf("Error: Can not open stream\n");
        return -1;
    }
    if(waitpid(child, NULL, 0) == -1){
        printf("Error: waitpid() error\n");
        return -1;
    }

    char buffer[SIZE];
    double result = 0.0;
    int counter = 0;
    ssize_t st;


    while(counter < n){
        st = read( fd, buffer, SIZE);
        if(st > 0)
        {
            buffer[st] = '\0';

            char * format_buffer = strtok(buffer, ";");
            while(format_buffer != NULL){
                result += get_double_number(format_buffer);
                format_buffer = strtok(NULL, ";");

                counter += 1;
            }

        }
    }
    close(fd);
    unlink(FIFO_PATH);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("======================================================================================\n");
    printf("Result of the integral with h = %.15f and n = %d is equal: %.15f\n", h, n, result);
    printf("Program running time: %.5f [s] \n", time);
    return 0;

}


void print_correct_format(){
    printf("Correct format: ./integral <rectangle_length> <process_number>\n");
}

int get_int_number(char * string_number){
    int number = atoi(string_number);

    if (number <= 0){
        return -1; // Error signal
    }

    return number;

}

double get_double_number(char * string_number){
    double num;
    sscanf(string_number, "%lf", &num);
    return num;

}

double min(double a, double b){
    return a < b ? a : b;
}
