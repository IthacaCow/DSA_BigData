target=bigData.o debug.o
CC=g++
CPPFLAGS=-march=native -Ofast -std=c++11 -fopenmp  -pthread -mcmodel=medium #-DDEBUG #-DBENCHMARK
LINKOPTION=-dynamic -mcmodel=medium -fopenmp

demo: main.cpp bigData.hpp
	$(CC) $(CPPFLAGS) main.cpp -o demo

debug_all: bigData debug
	$(CC) $(LINKOPTION) main.o debug.o -o bigData

bigData: main.cpp bigData.hpp
	$(CC) $(CPPFLAGS) -DBENCHMARK -DDEBUG -c main.cpp -o main.o

debug: debug.cpp
	$(CC) $(CPPFLAGS) -c debug.cpp -o debug.o

clean:
	rm -f *.o bigData demo

run:
	./bigData /tmp2/KDDCup2012/track2/kddcup2012track2.txt

d: all
	./bigData data_input



