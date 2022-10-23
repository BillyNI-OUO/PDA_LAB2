all: Lab1

CXX = g++

LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3

Lab1: main.o block.o
	$(CXX) $(LINKFLAGS) main.o -o Lab2

main.o: main.cpp
	$(CXX) $(LINKFLAGS) -DCOMPILETIME="\"`date`\"" main.cpp -c


block.o: ./src/block.cpp ./src/block.h
	$(CXX) $(LINKFLAGS) ./src/block.cpp -c

clean:
	rm -rf *.o *.gch Lab2






