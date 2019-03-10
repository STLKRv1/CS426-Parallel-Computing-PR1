
#include <stdio.h>
#include <mpi.h>
#include <malloc.h>

#define MASTER_TO_SLAVE_TAG 1   //tag for messages sent from master to slaves
#define SLAVE_TO_MASTER_TAG 10 //tag for messages sent from slaves to master

typedef struct dynamic_array_struct
{
    int* data;
    size_t capacity; /* total capacity */
    int size; /* number of elements in vector */
} vector;

int vector_init(vector* v, size_t init_capacity)
{
    v->data = malloc(init_capacity * sizeof(int));
    if (!v->data) return -1;

    v->size = 0;
    v->capacity = init_capacity;

    return 0;
}

int vector_free(vector* v)
{
    free(v->data);
    //free(v);

    return 0;
}
int vector_push_back(vector* v, int elem)
{
    if (v->size +1 >= v->capacity)
    {
        int * newdata = malloc(sizeof(int) * (v->capacity * 2 ));
        v->capacity *= 2;
        for(int i =0; i < v->size; i++)
        {
            newdata[i] = v->data[i];
        }
        free(v->data);
        v->data = newdata;
        newdata = NULL;
    }

    v->data[v->size] = elem;
    //printf("pushed: %d \n", v->data[v->size]);
    v->size = v->size+1;
    //printf("size: %d, capacity: %d \n", v->size, v->capacity);

    return 0;
}

int main(int argc, char *argv[] )
{
    //double timeStart, timeEnd;
    char* fileName;
    if(argc>=2)
    {
        fileName = argv[1];
    }
    char line[256];
    int partial_sum = 0;
    int global_sum = 0;
    int num=0;

    int myid, numprocs;
    MPI_Status s;
    MPI_Init(&argc,&argv);
    //timeStart = MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);


    if(myid == 0) {
        vector v;
        vector_init(&v, 10);

        FILE *file = fopen(fileName, "r");
        //read values into a vector
        while (fgets(line, sizeof(line), file))
        {
            sscanf(line, "%d", &num);
            //printf("%s\n", line);
            vector_push_back(&v, num);
        }
        fclose(file);

        int k;
        //pass values to the slave processes
        for (int i = 1; i < numprocs; i++)
        {
            if (i != numprocs - 1)
                k = (int) (v.size / numprocs);
            else
                k = v.size - ((int) (v.size / numprocs)) * (numprocs - 1);

            MPI_Send(&k, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            MPI_Send(&v.data[(int) v.size / numprocs * i], k, MPI_INT, i, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD);
        }

        //sum up the masters part
        for(int i = 0; i< (int)(v.size/numprocs); i++)
        {
            partial_sum += v.data[i];
        }

        MPI_Allreduce(&partial_sum, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        printf("[%d]Sum: %d \n", myid, global_sum);

        vector_free(&v);
    }

    //Slave processes
    else
    {
        //printf("[%d] reached %d\n", myid, pl++);
        int* nums;
        int qty = 0;
        MPI_Recv(&qty, 1, MPI_INT, 0, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &s);
        nums = malloc(sizeof(int) * qty);

        MPI_Recv(nums, qty, MPI_INT, 0, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD, &s);

        for(int j = 0; j < qty; j++)
            partial_sum += nums[j];

        MPI_Allreduce(&partial_sum, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        printf("[%d]Sum: %d \n", myid, global_sum);

        free(nums);
    }

    /*timeEnd = MPI_Wtime();
    printf ("[%d]Took %f seconds\n", myid, timeEnd - timeStart);*/
    MPI_Finalize();
    return 0;
}