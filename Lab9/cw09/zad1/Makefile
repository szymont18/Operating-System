CC = gcc
FLAGS = -Wall

compile: main.c
	${CC} ${FLAGS} -c main.c -lpthread
	${CC} ${FLAGS} main.o -o main -lpthread

clean:
	rm -rf *.o main