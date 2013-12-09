BIN=atomic_counter

CPP=g++-4.6

all: $(BIN)

$(BIN): atomic_counter.cpp
	$(CPP) -Wall -O3 -o $@ $< -std=c++0x -lpthread -lm

clean:
	rm -f $(BIN)
