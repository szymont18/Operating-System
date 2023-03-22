CC = gcc
FLAGS = -Wall


all: library.c classifier.c main.c
	make all_stat
	make all_shared
	make all_dynamic

library_stat: library.c
	${CC} ${FLAGS} -c library.c
	ar cr library.a library.o

classifier_stat: classifier.c
	${CC} ${FLAGS} -c classifier.c
	ar cr classifier.a classifier.o

all_stat: library.c classifier.c main.c library.o classifier.o
	make library_stat
	make classifier_stat
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o library.o classifier.o -o main_stat

library_shared: library.c
	${CC} ${FLAGS} -c -fPIC library.c
	${CC} -shared -fPIC -o liblibrary.so library.o

classifier_shared: classifier.c
	${CC} ${FLAGS} -c -fPIC classifier.c
	${CC} -shared -fPIC -o libclassifier.so classifier.o

all_shared: library.c classifier.c main.c library.o classifier.o
	make library_shared
	make classifier_shared

	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_shared main.c -Wl,-rpath=./ -L. -llibrary -lclassifier

all_dynamic: library.c classifier.c main.c library.o classifier.o
	make library_shared
	make classifier_shared
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_dynamic main.c -Wl,-rpath=./ -L. -ldl -D DYNAMIC

clean:
	rm -rf *o *a *so


