
CC = gcc
CFLAGS = -O3 -Wall
LIBRARIES = -lm -pthread

OBJECTS = test.o func.o linkedlist.o my_thread.o
PROGRAM = test

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBRARIES) -o $(PROGRAM)

clean:
	rm -f *.o $(PROGRAM)

