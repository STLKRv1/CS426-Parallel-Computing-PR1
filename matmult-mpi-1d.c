#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>

#define MASTER_TO_SLAVE_TAG 1 //tag for messages sent from master to slaves
#define SLAVE_TO_MASTER_TAG 10 //tag for messages sent from slaves to master


int read_matrix(int*** mat_ptr, int n, FILE* file)
{
    int num;
    int** mat;
    mat = (int**)malloc(sizeof(int*) * n);
    for(int i =0; i < n; i++)
        mat[i] = malloc(sizeof(int) * n);

    //read in the matrix
    int p=0;
    while ( fscanf (file, "%d", &num) == 1) {
        /*if(p%n==0)
            printf("\n");*/

        mat[(int)p/n][p%n] = num;
        //printf("%d ", mat[(int)p/n][p%n]);
        p++;
    }

    *mat_ptr = mat;
    //return 0;
}

int transpose_matrix(int** original, int*** res_ptr, int n)
{
    int** transposed = (int **)malloc(sizeof(int*) * n);

    for( int i = 0; i < n; i++)
    {
        transposed[i] = malloc(sizeof(int) * n);
        for ( int j = 0; j < n; j++ )
        {
            transposed[i][j] = original[j][i];
        }
    }

    *res_ptr = transposed;
}

int flatten(int** mat_2d, int** res_ptr, int n)
{
    int* res;
    res = (int*)malloc(sizeof(int) * n * n);

    for(int i = 0; i < n*n ; i++)
    {
        res[i] = mat_2d[i/n][i%n];
    }

    *res_ptr = res;

    return 0;
}

int main(int argc, char* argv[])
{
    //double timeStart, timeEnd;
    char* fileName;
    char* fileName2;
    char* fileName3;
    if(argc>=2)
    {
        fileName = argv[1];
        fileName2 = argv[2];
        fileName3 = argv[3];
    }

    char line[256];
    int n = 8;

    int myid, numprocs;
    MPI_Status s;
    MPI_Init(&argc,&argv);
    //timeStart = MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    int** mat;
    int** mat2;
    int** transm2;
    int** mat_res;
    int tile_size;
    int tile_count;

    //Master process
    if (myid == 0)
    {
        FILE* file = fopen(fileName, "r");
        fgets(line, 256, file);
        sscanf (line, "%d", &n);
        read_matrix(&mat, n, file);
        fclose(file);

        file= fopen(fileName2, "r");

        fgets(line, 256, file);
        sscanf (line, "%d", &n);

        read_matrix(&mat2, n, file);
        fclose(file);

        int* flat;
        int* flat2;
        //flatten 1st matrix
        flatten(mat, &flat, n);

        //transpose&flatten 2nd matrix
        transpose_matrix(mat2, &transm2, n);
        flatten(transm2, &flat2, n);

        //allocate result matrix
        mat_res = malloc(sizeof(int*) * n);
        for (int i = 0; i < n; i++)
        {
            mat_res[i] = malloc(sizeof(int) * n);
        }

        tile_size = (int)sqrt(n*n/numprocs);
        tile_count = n/tile_size;
        //Send data to slaves
        int x, y, sum;
        for(int i = 1; i < numprocs; i++)
        {
            x = (int)(i / tile_count);
            y = i % tile_count;
            /*printf("[%d] mat1: %d  mat2: %d \n", myid, flat[x*tile_size*n + (int)(n*n / numprocs)], flat2[y*tile_size*n] + (int)(n*n / numprocs));
            fflush(stdout);*/
            MPI_Send(&n, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            MPI_Send(&flat[x*tile_size*n], tile_size*n, MPI_INT, i, MASTER_TO_SLAVE_TAG +1, MPI_COMM_WORLD);
            MPI_Send(&flat2[y*tile_size*n], tile_size*n, MPI_INT, i, MASTER_TO_SLAVE_TAG +2, MPI_COMM_WORLD);
        }

        //Calculate masters part
        for(int i =0; i < n*n/numprocs; i++)
        {
            x = (int)(i / tile_size);
            y = i % tile_size;
            sum =0;

            for (int j = 0; j < n; ++j)
            {
                sum += flat[ y*n + j] * flat2[x*n + j];
            }
            mat_res[y][x] = sum;
        }


        int* partial_mat = malloc(sizeof(int) * n*n/numprocs);
        //recieve slaves parts
        for (int i = 1; i < numprocs; ++i)
        {
            MPI_Recv(partial_mat, (int)(n*n / numprocs), MPI_INT, i, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD, &s);
            for (int j = 0; j < (int)(n*n / numprocs); ++j)
            {
                x = (int)(i / tile_count)*tile_size + (int)(j / tile_size);
                y = (i % tile_count)*tile_size + j % tile_size;
                /*printf("[%d][%d] = %d\n", x,y, partial_mat[j]);
                fflush(stdout);*/
                mat_res[x][y] = partial_mat[j];
            }
        }


        file = fopen(fileName3, "w");
        fprintf(file, "%d", n);
        int res = 0;

        for (int i = 0; i < n; i++)
        {
            fprintf(file, "\n");
            for(int j = 0; j < n; j++)
            {
                fprintf(file, "%d ", mat_res[i][j]);
            }
        }

        for(int i =0; i < n; i++)
            free(mat[i]);
        free(mat);

        for(int i =0; i < n; i++)
            free(mat2[i]);
        free(mat2);
        for(int i =0; i < n; i++)
            free(transm2[i]);
        free(transm2);
        free(flat);
        free(flat2);
        free(partial_mat);

    }
    else
    {
        int sum = 0;
        int* par_mat1;
        int* par_mat2;
        int* par_mat_res;
        MPI_Recv(&n, 1, MPI_INT, 0, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &s);
        tile_size = (int)sqrt(n*n/numprocs);
        par_mat1 = malloc(sizeof(int) * tile_size*n);
        par_mat2 = malloc(sizeof(int) * tile_size*n);
        par_mat_res = malloc(sizeof(int) * (n*n / numprocs));

        MPI_Recv(par_mat1, tile_size*n, MPI_INT, 0, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD, &s);
        MPI_Recv(par_mat2, tile_size*n, MPI_INT, 0, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD, &s);

        for (int i = 0; i < (int)(n*n / numprocs); ++i)
        {
            int x = ((int)i/tile_size);
            int y = i%tile_size;
            sum = 0;

            /*printf("[%d] x: %d  y: %d \n", myid, x, y);
            fflush(stdout);*/
            for (int j = 0; j < n; ++j)
            {
                /*printf("[%d] mat1: %d  mat2: %d \n", myid, par_mat1[x * n + j], par_mat2[y * n + j]);
                fflush(stdout);*/
                sum += par_mat1[x * n + j] * par_mat2[y * n + j];
            }

            par_mat_res[i] = sum;
        }

        MPI_Send(par_mat_res, (int)(n*n / numprocs), MPI_INT, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD);

        free(par_mat1);
        free(par_mat2);
        free(par_mat_res);
    }


    /*file = fopen(fileName3, "w");
    fprintf(file, "%d", n);
    int res = 0;

    for (int i = 0; i < n; i++)
    {
        fprintf(file, "\n");
        for(int j = 0; j < n; j++)
        {
            for(int k = 0; k < n ; k++)
            {
                res += mat[i][k] * mat2[k][j];
            }
            fprintf(file, "%d ", res);
            res=0;
        }
    }*/

    /*timeEnd = MPI_Wtime();
    printf ("[%d]Took %f seconds\n", myid, timeEnd - timeStart);*/
    MPI_Finalize();
    return 0;
}