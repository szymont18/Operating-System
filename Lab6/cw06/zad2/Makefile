CC = gcc
FLAGS = -Wall

compile: server.c client.c
	${CC} ${FLAGS} -c server.c
	${CC} ${FLAGS} server.o -o server -lrt
	${CC} ${FLAGS} -c client.c
	${CC} ${FLAGS} client.o -o client -lrt


clean:
	rm -rf *.o server client