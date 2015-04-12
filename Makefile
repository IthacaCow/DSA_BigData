target=bigData.o debug.o
CC=g++
CPPFLAGS=-O3 -Wall -g -std=c++11   -pthread -mcmodel=medium #-DDEBUG
LINKOPTION=-dynamic -mcmodel=medium 

all: bigData debug
	$(CC) main.o debug.o -o bigData

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



