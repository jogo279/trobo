CXXFLAGS=-Wall -Wextra -Wno-deprecated -O3 -g -funroll-loops # -pg

all: MyTronBot

MyTronBot: MyTronBot.o
	g++ $(CXXFLAGS) -o MyTronBot MyTronBot.o

MyTronBot.o: MyTronBot.cc Map.h
	g++ $(CXXFLAGS) -c -o MyTronBot.o MyTronBot.cc

clean:
	rm -f *.o MyTronBot

.PHONY: clean