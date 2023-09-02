CC = gcc
FLAGS = -Wall

compile:client.c server.c
	${CC} ${FLAGS} -c client.c -lpthread
	${CC} ${FLAGS} client.o -o client -lpthread
	${CC} ${FLAGS} -c server.c -lpthread
	${CC} ${FLAGS} server.o -o server -lpthread


clean:
	rm -rf *.o server client my_server

