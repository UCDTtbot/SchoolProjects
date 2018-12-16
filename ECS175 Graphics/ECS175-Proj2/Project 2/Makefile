# Tyler Welsh, 912341695, ECS 175 Project 1

CC = g++
CFLAGS = -c -Wall -g -lglut -lGL
LDFLAGS = -g -lglut -lGL

all: out

out: main.o 3DPolygon.o
	$(CC) main.o 3DPolygon.o $(LDFLAGS) -o out

3Dpolygon.o: 3DPolygon.cpp 3DPolygon.h
	$(CC) $(CFLAGS) 3DPolygon.cpp -o 3DPolygon.o

main.o: main.cpp 3DPolygon.h
	$(CC) $(CFLAGS) main.cpp -o main.o

3Dpolygon.h: 3DPoint.h Edge.h

Edge.h: 3DPoint.h

3DPoint.h: 2DPoint.h

	
clean:
	rm -f *.o *~ core
