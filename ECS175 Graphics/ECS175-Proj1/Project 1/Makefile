# Tyler Welsh, 912341695, ECS 175 Project 1

CC = g++
CFLAGS = -c -Wall -g -lglut -lGL
LDFLAGS = -g -lglut -lGL

all: out

out: main.o polygon.o
	$(CC) main.o polygon.o $(LDFLAGS) -o out

polygon.o: polygon.cpp polygon.h
	$(CC) $(CFLAGS) polygon.cpp -o polygon.o

main.o: main.cpp polygon.h
	$(CC) $(CFLAGS) main.cpp -o main.o

polygon.h: 2DPoint.h Edge.h

Edge.h: 2DPoint.h

2DPoint.h: Window.h
	
clean:
	rm -f *.o *~ core
