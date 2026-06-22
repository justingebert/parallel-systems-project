CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -fopenmp -Ilib
TARGET := rubiks-core
SIMPLE_TARGET := cube-simple
SOURCES := main.c cube.c solve.c benchmark.c lib/cJSON.c

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


## MPI 
MPICC ?= mpicc
NP ?= 4
MPI_TARGET := rubiks-mpi
MPI_SOURCES := main.c cube.c solve.c solve_mpi.c benchmark.c lib/cJSON.c

mpi: $(MPI_TARGET)

$(MPI_TARGET): $(MPI_SOURCES) cube.h solve_mpi.h
	$(MPICC) $(CFLAGS) $(MPI_SOURCES) -o $(MPI_TARGET)

run-mpi: mpi
	mpirun --allow-run-as-root -np $(NP) ./$(MPI_TARGET)

clean:
	rm -f $(TARGET) $(SIMPLE_TARGET) $(MPI_TARGET)

.PHONY: build run simple run-simple mpi run-mpi clean
