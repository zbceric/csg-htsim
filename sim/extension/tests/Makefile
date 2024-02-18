CC = g++
CFLAGS = -Wall -std=c++11 -g -Wsign-compare
CRT=`pwd`
INCLUDE= -I/$(CRT)/../.. -I/$(CRT)/.. -I$(CRT) 
LIB=-L../.. -L../../extension
DEPS=../../libhtsim.a ../../extension/libextension.a

all: test_progress

main_progress.o: main_progress.cpp ${DEPS}
	$(CC) $(INCLUDE) $(CFLAGS) -c main_progress.cpp 

test_progress: main_progress.o ../../libhtsim.a
	$(CC) $(CFLAGS) main_progress.o $(LIB) -lhtsim -lextension -o test_progress

clean:	
	rm -f *.o test_*
