#
# Makefile for the simple-race example.
#

CXX = icpc
CXXFLAGS = -O3 -g

TARGET = simple-race
SRC = simple-race.cpp

.PHONY: all
all: $(TARGET)


$(TARGET): $(SRC)
	$(CXX) -o $@ $(CXXFLAGS) $^

.PHONY: clean
clean:
	rm -f $(TARGET)
