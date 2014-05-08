#
# Makefile for the qsort-race example.
#

CXX = icpc
CXXFLAGS = -O3 -g

TARGET = qsort-race
SRC = qsort-race.cpp

.PHONY: all
all: $(TARGET)


$(TARGET): $(SRC)
	$(CXX) -o $@ $(CXXFLAGS) $^

.PHONY: clean
clean:
	rm -f $(TARGET)
