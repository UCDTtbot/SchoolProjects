# Tyler Welsh, 912341695, ECS 175 Project 1

CFLAGS += -c -Wall -g -lglut -lGL
LDFLAGS = -g -lglut -lGL

CC = g++

all: curves

curves: main.o BCurve.o
	$(CC) main.o BCurve.o $(LDFLAGS) -o curves

BCurve.o: BCurve.cpp BCurve.h 
	$(CC) $(CFLAGS) BCurve.cpp -o BCurve.o

main.o: main.cpp 2DPolygon.h BCurve.h
	$(CC) $(CFLAGS) main.cpp -o main.o

3Dpolygon.h: 2DPoint.h Edge.h

Edge.h: 2DPoint.h

clean:
	rm -f *.o *~ core
