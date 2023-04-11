CC = gcc
FLAGS = -Wall
OUTPUT = raport.txt
SIZE = 1 2 3 5 10 50 100 500 1000
TEST1 = 0.0000001
TEST2 = 0.0000000001

all: integral.c
	make clean
	make compile

compile: integral.c
	${CC} ${FLAGS} -c integral.c
	${CC} ${FLAGS} integral.o -o integral


clean:
	rm -rf *.so *.o integral

test: integral.c
	make all
	touch ${OUTPUT}
	for i in ${SIZE}; do \
    		./integral ${TEST1} $$i >> ${OUTPUT}; \
    	done
	echo "\n \n \n" >> ${OUTPUT}

	for i in ${SIZE}; do \
          ./integral ${TEST2} $$i >> ${OUTPUT}; \
        done






