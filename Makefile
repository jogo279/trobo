# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CXXFLAGS +=-g -lboost_program_options

MyTronBot: MyTronBot.o Map.o
	g++ $(CXXFLAGS) -o MyTronBot MyTronBot.o Map.o
	
MyTronBot.o: MyTronBot.cc
	g++ $(CXXFLAGS) -c -o MyTronBot.o MyTronBot.cc

Map.o: Map.cc
	g++ $(CXXFLAGS) -c -o Map.o Map.cc

clean: 
	rm -f MyTronBot MyTronBot.o Map.o
