CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -fopenmp
TARGET := rubiks-core
SIMPLE_TARGET := cube-simple
SOURCES := main.c cube.c

build: $(TARGET)

$(TARGET): $(SOURCES) cube.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: build
	./$(TARGET)

simple: $(SIMPLE_TARGET)

$(SIMPLE_TARGET): cube-simple.c
	$(CC) $(CFLAGS) cube-simple.c -o $(SIMPLE_TARGET)

run-simple: simple
	./$(SIMPLE_TARGET)

clean:
	rm -f $(TARGET) $(SIMPLE_TARGET)

.PHONY: build run simple run-simple clean
