CC = gcc
FLAGS = -Wall
FILE_IN = "lib.c"
FILE_OUT = "out.txt"
LIB = "lib_"
SYS = "sys_"

all: lib.c sys.c
	make compile

compile: lib.c sys.c
	${CC} ${FLAGS} -c lib.c
	${CC} ${FLAGS} lib.o -o lib

	${CC} ${FLAGS} -c sys.c
	${CC} ${FLAGS} sys.o -o sys

clean:
	rm -rf *o *a *so
	rm -rf tmp.txt *out.txt lib sys

test: lib.c sys.c
	./lib " " "k" ${FILE_IN} "${LIB}${FILE_OUT}"
	./sys " " "k" ${FILE_IN} "${SYS}${FILE_OUT}"
	tr " " "k" < ${FILE_IN} > tmp.txt
	diff -s tmp.txt "${LIB}${FILE_OUT}"
	diff -s tmp.txt "${SYS}${FILE_OUT}"


