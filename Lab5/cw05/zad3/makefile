CC = gcc
FLAGS = -Wall
OUTPUT = raport.txt
SIZE = 1 2 3 5 10 50 100
TEST1 = 0.0000001
TEST2 = 0.0000000001

all: integral.c main.c
	make clean
	make compile

compile: integral.c main.c
	${CC} ${FLAGS} -c integral.c
	${CC} ${FLAGS} integral.o -o integral
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o -o main

clean:
	rm -rf *.so *.o integral main integral_stream

test: integral.c main.c
	make all
	touch ${OUTPUT}
	for i in ${SIZE}; do \
    		./main ${TEST1} $$i >> ${OUTPUT}; \
    	done
	echo "\n \n \n" >> ${OUTPUT}

	for i in ${SIZE}; do \
          ./main ${TEST2} $$i >> ${OUTPUT}; \
        done
