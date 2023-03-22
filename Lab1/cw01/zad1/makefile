CC = gcc
FLAGS = -Wall

all:
	make static_lib
	make shared_lib

static_lib: library.c
	${CC} ${FLAGS} -c library.c
	ar cr library.a library.o

shared_lib: library.c
	${CC} ${FLAGS} -c -fPIC library.c
	${CC} ${FLAGS} -shared -fPIC -o library.so library.o

clean:
	rm -rf *o
	rm -rf *a




