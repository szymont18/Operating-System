#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>


int SIZE = PIPE_BUF; // from limits.h


// Functions declarations
double get_double_number(char * string_number);
double integral_function(double x);
double calculate_integral(double left, double right, double h);
double min(double a, double b);

int main(int argc, char ** argv){
    double left = get_double_number(argv[1]);
    double right = get_double_number(argv[2]);
    double h = get_double_number(argv[3]);
    char * stream_path = argv[4];

    double result = calculate_integral(left, right, h);
    char buffer[SIZE];
    memset(buffer, 0, sizeof (buffer));
    snprintf(buffer, SIZE, "%.15f;", result);

    int fd = open(stream_path, O_WRONLY);
    if (fd == -1){
        perror("");
        return -1;
    }

    write(fd, buffer, strlen(buffer));
    close(fd);
    return 0;

}

double get_double_number(char * string_number){
    double num;
    sscanf(string_number, "%lf", &num);
    return num;
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


