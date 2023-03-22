CC = gcc
FLAGS = -Wall
DIR = .

all: main.c
	make compile
	make test

compile: main.c
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *o *a *so main

test: main.c
	./main .





