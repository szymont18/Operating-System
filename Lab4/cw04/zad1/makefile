CC = gcc
FLAGS = -Wall

all:
	make compile

compile: main.c exec_function.c
	${CC} ${FLAGS} -c exec_function.c
	${CC} ${FLAGS} exec_function.o -o exec_function
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *o *a *so main exec_function


