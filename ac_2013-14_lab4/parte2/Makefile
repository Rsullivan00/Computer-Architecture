CC = g++
CFLAGS= -Wall -g -std=c++0x
PRGS = histogram_atom histogram_mutex
AUX = img_aux.cpp
LIBS = -lm -pthread
OBJS = $(PRGS)

all: $(OBJS)

$(PRGS): 
	$(CC) $(CFLAGS) -o $@ $@.cpp $(AUX) $(LDFLAGS) $(LIBS)

clean:
	-rm -f *.o  *~ $(PRGS) $(OMPPRGS)
