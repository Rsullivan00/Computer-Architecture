BIN=mutex_counter

CPP=g++-4.6

all: $(BIN)

$(BIN): mutex_counter.cpp
	$(CPP) -Wall -O3 -o $@ $< -std=c++0x -lpthread -lm

clean:
	rm -f $(BIN)
