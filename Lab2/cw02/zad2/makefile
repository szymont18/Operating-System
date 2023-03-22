CC = gcc
FLAGS = -Wall
FILE_IN = "block_rev.c"
FILE_OUT = "out.txt"
BLOCK = "block_"
CHAR = "char_"

all: block_rev.c char_rev.c
	make compile

compile: block_rev.c char_rev.c
	${CC} ${FLAGS} -c block_rev.c
	${CC} ${FLAGS} block_rev.o -o block_rev
	${CC} ${FLAGS} -c char_rev.c
	${CC} ${FLAGS} char_rev.o -o char_rev

clean:
	rm -rf *o *a *so
	rm -rf *_block.txt *_char.txt
	rm -rf *${FILE_OUT} block_rev char_rev

test: block_rev.c char_rev.c
	./block_rev ${FILE_IN} "${BLOCK}${FILE_OUT}"
	./char_rev ${FILE_IN} "${CHAR}${FILE_OUT}"

	./block_rev "${BLOCK}${FILE_OUT}" "${BLOCK}${BLOCK}${FILE_OUT}"
	./block_rev "${CHAR}${FILE_OUT}" "${BLOCK}${CHAR}${FILE_OUT}"

	./char_rev "${BLOCK}${FILE_OUT}" "${CHAR}${BLOCK}${FILE_OUT}"
	./block_rev "${CHAR}${FILE_OUT}" "${CHAR}${CHAR}${FILE_OUT}"


	diff -s "${BLOCK}${BLOCK}${FILE_OUT}" ${FILE_IN}
	diff -s "${BLOCK}${CHAR}${FILE_OUT}" ${FILE_IN}
	diff -s "${CHAR}${BLOCK}${FILE_OUT}" ${FILE_IN}
	diff -s "${CHAR}${CHAR}${FILE_OUT}" ${FILE_IN}











