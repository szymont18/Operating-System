CC = gcc
FLAGS = -Wall
FILE_IN = "lorem_ipsum"
FILE_OUT = "out.txt"
BLOCK = "block_"
CHAR = "char_"

all: main.c
	make compile
	./main

compile: main.c
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *o *a *so
	rm -rf *_block.txt *_char.txt
	rm -rf *${FILE_OUT} main

test:
	wc --bytes *





