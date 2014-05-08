# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CILK=/afs/andrew/usr7/ericwong/cilkplus-4_8-install/bin/g++ -fcilkplus -lcilkrts
#TBB=-I tbb42_20140122oss/include -L tbbzip/lib -ltbb

CXXFLAGS +=-O2 -lboost_program_options -std=c++11

MyTronBot: MyTronBot.o Map.o Abort.o ABState.o
	$(CILK) $(CXXFLAGS) $(TBB) -o MyTronBot MyTronBot.o Map.o Abort.o ABState.o
	
MyTronBot.o: MyTronBot.cc
	$(CILK) $(CXXFLAGS) $(TBB) -c -o MyTronBot.o MyTronBot.cc

Map.o: Map.cc
	$(CILK) $(CXXFLAGS) $(TBB) -c -o Map.o Map.cc

Abort.o: Abort.cc
	$(CILK) $(CXXFLAGS) $(TBB) -c -o Abort.o Abort.cc

ABState.o: ABState.cc
	$(CILK) $(CXXFLAGS) $(TBB) -c -o ABState.o ABState.cc

clean: 
	rm -f MyTronBot MyTronBot.o Map.o Abort.o ABState.o
