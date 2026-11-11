CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
SOURCES := MAIN.cpp Game.cpp Renderer.cpp Terminal.cpp
HEADERS := Game.hpp Renderer.hpp Terminal.hpp
TEST_SOURCES := tests/game_tests.cpp Game.cpp Renderer.cpp

.PHONY: all run test clean

all: snake

snake: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o snake

run: snake
	./snake

snake_tests: $(TEST_SOURCES) Game.hpp Renderer.hpp
	$(CXX) $(CXXFLAGS) -I. $(TEST_SOURCES) -o snake_tests

test: snake_tests
	./snake_tests

clean:
	rm -f snake snake_tests
