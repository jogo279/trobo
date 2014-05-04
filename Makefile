# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CILK=/afs/andrew/usr7/ericwong/cilkplus-4_8-install/bin/g++ -fcilkplus -lcilkrts

CXXFLAGS +=-g -lboost_program_options -std=c++11

MyTronBot: MyTronBot.o Map.o
	$(CILK) $(CXXFLAGS) -o MyTronBot MyTronBot.o Map.o
	
MyTronBot.o: MyTronBot.cc
	$(CILK) $(CXXFLAGS) -c -o MyTronBot.o MyTronBot.cc

Map.o: Map.cc
	$(CILK) $(CXXFLAGS) -c -o Map.o Map.cc

clean: 
	rm -f MyTronBot MyTronBot.o Map.o
