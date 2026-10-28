CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Isource

all: analytics tests_run benchmark

analytics: source/main.cpp
	$(CXX) $(CXXFLAGS) source/main.cpp -o analytics

tests_run: tests/tests.cpp
	$(CXX) $(CXXFLAGS) tests/tests.cpp -o tests_run

benchmark: tests/benchmark.cpp
	$(CXX) $(CXXFLAGS) tests/benchmark.cpp -o benchmark

run: analytics
	./analytics

demo: analytics
	./analytics --demo

test: tests_run
	./tests_run

bench: benchmark
	./benchmark

clean:
	rm -f analytics tests_run benchmark *.exe
