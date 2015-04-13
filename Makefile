target=bigData.o debug.o
CC=g++
CPPFLAGS=-march=native -Ofast -Wall -std=c++11 -fopenmp  -pthread -mcmodel=medium #-DBENCHMARK#-DDEBUG LINKOPTION=-dynamic -mcmodel=medium -fopenmp

all: bigData debug
	$(CC) $(LINKOPTION) main.o debug.o -o bigData2

bigData: main.cpp bigData.hpp
	$(CC) $(CPPFLAGS) -c main.cpp -o main.o

debug: debug.cpp
	$(CC) $(CPPFLAGS) -c debug.cpp -o debug.o

clean:
	rm -f *.o bigData

run:
	./bigData /tmp2/KDDCup2012/track2/kddcup2012track2.txt

d: all
	./bigData data_input



