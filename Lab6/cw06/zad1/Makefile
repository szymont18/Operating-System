CC = gcc
FLAGS = -Wall

compile: server.c
	${CC} ${FLAGS} -c server.c
	${CC} ${FLAGS} server.o -o server
	${CC} ${FLAGS} -c client.c
	${CC} ${FLAGS} client.o -o client

clean:
	rm -rf *.o server client