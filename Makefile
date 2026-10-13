CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

passguard: src/main.cpp src/checker.cpp src/generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test_passguard: tests/test_passguard.cpp src/checker.cpp src/generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test: test_passguard
	./test_passguard

clean:
	rm -f passguard test_passguard

.PHONY: test clean
