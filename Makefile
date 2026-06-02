ifeq ($(shell uname),Darwin)
export OMPI_CC := gcc-15
endif

build:
	mpicc -fopenmp hello.c -o hello

run:
	mpirun -n 4 ./hello

clean:
	rm -f hello

.PHONY: build run clean
