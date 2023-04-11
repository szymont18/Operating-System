#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Integrals Boundaries
double A = 0.0;
double B = 1.0;

// Functions declarations
void print_correct_format();
int get_int_number(char * string_number);
double get_double_number(char * string_number);
double integral_function(double x);
double calculate_integral(double left, double right, double h);
double min(double a, double b);
double get_result(int * descriptors, int n);

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
    // Measure time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int read_descriptors[n];
    double left_boundary = 0.0, step = (B - A) / n;
    pid_t child_pid;

    for (int i = 0; i < n; i++){

        int fd[2]; // File descriptors are coppied for all children process
        pipe(fd); // fd[0] - read; fd[1] - write

        child_pid = fork();

        if(child_pid == 0){
            close(fd[0]);
            double component = calculate_integral(left_boundary, min(left_boundary + step, B), h);
            char message[30];
            snprintf(message, sizeof(message), "%.15f", component);
            write(fd[1], message, sizeof(message));
            exit(0);
        }

        read_descriptors[i] = fd[0];
        left_boundary += step;
    }

//    Wait for last children
    if(waitpid(child_pid, NULL, 0) == -1){
        printf("Error: waitpid() error\n");
        return -1;
    }

    double result = get_result(read_descriptors, n);

//    Meassure time
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
    double number = strtod(string_number, NULL);
    if (number <= 0){
        return -1; // Error signal
    }

    return number;

}

double integral_function(double x){
    return 4/(x * x + 1);
}

double calculate_integral(double left, double right, double h){
    double result = 0.0;
    while (left + h < right){
        double middle = (left + h + left) / 2.0;
        double y_val = integral_function(middle);

        result += y_val;
        left = left + h;
    }

    result *= h;

    double last_middle = (left + right) / 2.0;
    double last_y_val = integral_function(last_middle);

    result += (last_y_val * (right - left)); // The last interval may be different from h

    return result;

}

double min(double a, double b){
    return a < b ? a : b;
}

double get_result(int * descriptors, int n){
    double result = 0.0;
    for (int i = 0; i < n; i++){
        char buffer[30];
        read(descriptors[i], buffer, sizeof (buffer));
        result += get_double_number(buffer);
    }
    return result;

}

