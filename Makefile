CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET := rubiks-core
SOURCES := main.c cube.c

build: $(TARGET)

$(TARGET): $(SOURCES) cube.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: build run clean
