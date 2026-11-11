CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

.PHONY: all run clean

all: snake

snake: MAIN.cpp
	$(CXX) $(CXXFLAGS) MAIN.cpp -o snake

run: snake
	./snake

clean:
	rm -f snake
