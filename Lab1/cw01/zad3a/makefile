CC = gcc
FLAGS = -Wall

all:
	make test_stat
	make test_shared
	make test_dynamic

report: results_dynamic.txt results_shared.txt results_static.txt
	make all
	echo "-------------STATIC LIBRARY-------------" > report.txt
	cat results_static.txt >> report.txt
	echo "-------------SHARED LIBRARY-------------" >> report.txt
	cat results_shared.txt >> report.txt
	echo "-------------DYNAMIC LIBRARY-------------" >> report.txt
	cat results_dynamic.txt >> report.txt
	sed -i 's/(null)/ /g' report.txt
	sed -i 's/_/ /g' report.txt

library_stat: library.c
	${CC} ${FLAGS} -c library.c
	ar cr library.a library.o

classifier_stat: classifier.c
	${CC} ${FLAGS} -c classifier.c
	ar cr classifier.a classifier.o

all_stat: library.c classifier.c main.c
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

all_shared: library.c classifier.c main.c
	make library_shared
	make classifier_shared

	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_shared main.c -Wl,-rpath=./ -L. -llibrary -lclassifier

all_dynamic: library.c classifier.c main.c
	make library_shared
	make classifier_shared
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_dynamic main.c -Wl,-rpath=./ -L. -ldl -D DYNAMIC


test_stat: library.c classifier.c main.c 
	make all_stat
	./main_stat < commands.txt > results_static.txt

test_shared: library.c classifier.c main.c
	make all_shared
	./main_shared < commands.txt > results_shared.txt

test_dynamic: library.c classifier.c main.c
	make all_dynamic
	./main_dynamic < commands.txt > results_dynamic.txt

clean:
	rm -rf *o *a *so
