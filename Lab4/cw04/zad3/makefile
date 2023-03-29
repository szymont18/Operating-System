CC = gcc
FLAGS = -Wall

make all:
	make compile

compile: catcher.c sender.c
	${CC} ${FLAGS} -c catcher.c
	${CC} ${FLAGS} catcher.o -o catcher
	${CC} ${FLAGS} -c sender.c
	${CC} ${FLAGS} sender.o -o sender

clean:
	rm -rf *o *a *so catcher sender
