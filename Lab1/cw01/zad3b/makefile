CC = gcc
FLAGS = -Wall

all:
	make clean
	make opt_stat
	make opt_shared
	make opt_dynamic

report: results_dynamic_O1.txt  results_shared_O0.txt  results_shared_Os.txt  results_static_O1.txt results_dynamic_O0.txt  results_dynamic_Os.txt  results_shared_O1.txt  results_static_O0.txt  results_static_Os.txt
	make all
	echo "STATIC LIBRARY" > report.txt
	echo "OPTIMIZATION O0" >> report.txt
	cat results_static_O0.txt >> report.txt
	echo "OPTIMIZATION O1" >> report.txt
	cat results_static_O1.txt >> report.txt
	echo "OPTIMIZATION Os" >> report.txt
	cat results_static_Os.txt >> report.txt

	echo "SHARED LIBRARY" >> report.txt
	echo "OPTIMIZATION O0" >> report.txt
	cat results_shared_O0.txt >> report.txt
	echo "OPTIMIZATION O1" >> report.txt
	cat results_shared_O1.txt >> report.txt
	echo "OPTIMIZATION Os" >> report.txt
	cat results_shared_Os.txt >> report.txt

	echo "DYNAMIC LIBRARY" >> report.txt
	echo "OPTIMIZATION O0" >> report.txt
	cat results_dynamic_O0.txt >> report.txt
	echo "OPTIMIZATION O1" >> report.txt
	cat results_dynamic_O1.txt >> report.txt
	echo "OPTIMIZATION Os" >> report.txt
	cat results_dynamic_Os.txt >> report.txt


	sed -i 's/(null)/ /g' report.txt
	sed -i 's/_/ /g' report.txt


library_stat: library.c
	${CC} ${FLAGS} -c library.c
	ar cr library.a library.o

classifier_stat: classifier.c
	${CC} ${FLAGS} -c classifier.c
	ar cr classifier.a classifier.o

library_shared: library.c
	${CC} ${FLAGS} -c -fPIC library.c
	${CC} -shared -fPIC -o liblibrary.so library.o

classifier_shared: classifier.c
	${CC} ${FLAGS} -c -fPIC classifier.c
	${CC} -shared -fPIC -o libclassifier.so classifier.o

opt_stat: library.c classifier.c main.c
	make library_stat
	make classifier_stat
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} main.o library.o classifier.o -o main_stat
	./main_stat < commands.txt > results_static_O0.txt

	${CC} ${FLAGS} main.o library.o classifier.o -o main_stat -O1
	./main_stat < commands.txt > results_static_O1.txt

	${CC} ${FLAGS} main.o library.o classifier.o -o main_stat -Os
	./main_stat < commands.txt > results_static_Os.txt

opt_shared: library.c classifier.c main.c
	make library_shared
	make classifier_shared
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_shared main.c -Wl,-rpath=./ -L. -llibrary -lclassifier
	./main_shared < commands.txt > results_shared_O0.txt

	${CC} ${FLAGS} -o main_shared main.c -Wl,-rpath=./ -L. -llibrary -lclassifier -O1
	./main_shared < commands.txt > results_shared_O1.txt

	${CC} ${FLAGS} -o main_shared main.c -Wl,-rpath=./ -L. -llibrary -lclassifier -Os
	./main_shared < commands.txt > results_shared_Os.txt

opt_dynamic: library.c classifier.c main.c
	make library_shared
	make classifier_shared
	${CC} ${FLAGS} -c main.c
	${CC} ${FLAGS} -o main_dynamic main.c -Wl,-rpath=./ -L. -ldl -D DYNAMIC
	./main_dynamic < commands.txt > results_dynamic_O0.txt

	${CC} ${FLAGS} -o main_dynamic main.c -Wl,-rpath=./ -L. -ldl -D DYNAMIC -O1
	./main_dynamic < commands.txt > results_dynamic_O1.txt

	${CC} ${FLAGS} -o main_dynamic main.c -Wl,-rpath=./ -L. -ldl -D DYNAMIC -Os
	./main_dynamic < commands.txt > results_dynamic_Os.txt

clean:
	rm -rf *o *a *so
