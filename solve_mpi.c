#include <mpi.h>
#include <stdio.h>

#include "solve_mpi.h"
#include "cube.h"

bool initMpi(CubeState cube, int length) {
  int rank, size;
  const int maxnamelen = MPI_MAX_PROCESSOR_NAME;
  int resultlen = maxnamelen;

  char name[maxnamelen];

  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Get_processor_name(name, &resultlen);


  if (rank == 0) {
    CubeExpansion expansion = expand(cube);
    int chunk = CUBE_MOVE_COUNT + size / size;
    int total = chunk * size;

    
  } else {
    
  }

  printf("Hello World from process %d of %d, and my name is %s\n", rank, size, name);
  
  MPI_Finalize();
}

