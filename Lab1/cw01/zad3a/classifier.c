//
// Created by szymek on 06.03.23.
//
#include "stdio.h"
#include <stdlib.h>
#include "classifier.h"
#include "string.h"
#include "library.h"

int classify(char * first, char * second, char *third, char *fourth){

    //Choose option
    if (first != NULL && first[strlen(first) - 1] == '\n') first[strlen(first) - 1] = 0;
    if (second != NULL && second[strlen(second) - 1] == '\n') second[strlen(second) - 1] = 0;
    if (third != NULL && third[strlen(third) - 1] == '\n') third[strlen(third) - 1] = 0;
    if (fourth != NULL && fourth[strlen(fourth) - 1] == '\n') fourth[strlen(first) - 1] = 0;

    if(fourth != NULL) return NOT_RECOGNIZED;


    if(strcmp(first, "help") == 0){
        if (second == NULL) return HELP;
        else{
            return NOT_RECOGNIZED;
        }
    }
    else if (strcmp(first, "init") == 0){
        int correct_capacity = atoi(second);

        if(correct_capacity == 0) return NOT_RECOGNIZED;

        if(third != NULL) return NOT_RECOGNIZED;

        return INIT;
    }

    else if(strcmp(first, "count") == 0){
        if(third != NULL) return NOT_RECOGNIZED;
        return COUNT;
    }

    else if (strcmp(first, "show") == 0){
        if (third != NULL) return NOT_RECOGNIZED;
        int index = atoi(second);
        if (strcmp(second, "0") != 0 && index == 0) return NOT_RECOGNIZED;

        return SHOW;
    }

    else if (strcmp(first, "delete") == 0){
        //(???) Maybe better comand delete index instead delete index index
        if(strcmp(second, "index") != 0) return NOT_RECOGNIZED;

        if(fourth != NULL) return NOT_RECOGNIZED;

        int index = atoi(third);

        if (strcmp(third, "0") != 0 && index == 0) return NOT_RECOGNIZED;

        return DELETE;
    }

    else if (strcmp(first, "destroy") == 0){
        if (second != NULL) return NOT_RECOGNIZED;
        return DESTROY;
    }

    else if(strcmp(first, "exit") == 0){
        if (second != NULL) return NOT_RECOGNIZED;
        return EXIT;
    }

    return NOT_RECOGNIZED;

}