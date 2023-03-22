CC = gcc
FLAGS = -Wall
DIR = "."
TEST = "result.txt"

all: main.c
	make compile
	./main ${DIR}


compile: main.c
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *o *a *so main


test:
	find ${DIR}  | xargs wc --bytes






