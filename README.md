# CS426-Parallel-Computing-PR1
2 part assigment that requested simple multi-threaded programs with Open-MPI. First part: reads numbers from a file and prints the sum. Second part: Matrix Multiplication. The serial implementations are used as a base-line for benchmarks. Report that explains the implementation decsisions and comments is also present in this repo.

To compile:
-c files *without* "mpi" use gcc command in linux.
-c files *with* "mpi" use mpicc or gcc with -lmpi command in linux.
note: matmult-mpi-1d.c requires -lm linker aswell.

To execute:
-Sum programs take 1 parameter for the file directory where the line seperated numbers are present.
-Matrix programs take 3 parameters file directory for matrices to be multiplied(2) and another file directory for result matrix to be written. The files that contain matrices start with an integer that specifies the dimesions of the square matrix and from the next line spaces denote columns new lines denote rows.
