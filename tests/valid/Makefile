PR = valid
CC = g++
MFLAGS = -o $(PR)
CFLAGS = -g -Wall -Os
LFLAGS =
REMOVE = rm -f

OBJECTS = main.o system.o

all: $(PR)

$(PR): $(OBJECTS)
	$(CC) $(MFLAGS) $(OBJECTS) $(LFLAGS)
    
%.o: %.cpp
	$(CC) -c $(CFLAGS) $<


clean:
	$(REMOVE) *.o *~ $(PR)

