CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -I.

SOURCES  = tests/test_all.cpp LinkedList.cpp Polynom.cpp
TARGET   = tests/test_all

.PHONY: all test demo clean

all: test

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

demo: radix_demo
	./radix_demo

radix_demo: Radix.cpp queue.h Radix.h
	$(CXX) $(CXXFLAGS) Radix.cpp -o $@

clean:
	rm -f $(TARGET) $(TARGET).exe radix_demo radix_demo.exe
