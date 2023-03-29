CC = gcc
FLAGS = -Wall

all: main.c
	make compile

compile: main.c
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *o *a *so main