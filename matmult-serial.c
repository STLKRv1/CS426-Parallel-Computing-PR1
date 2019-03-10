
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

int main(int argc, char* argv[])
{

    //clock_t start = clock();
    char* fileName;
    char* fileName2;
    char* fileName3;
    if(argc>=2)
    {
        fileName = argv[1];
        fileName2 = argv[2];
        fileName3 = argv[3];
    }
    FILE* file = fopen(fileName, "r");
    char line[256];
    int n;

    fgets(line, 256, file);
    sscanf (line, "%d", &n);

    int** mat;
    int** mat2;

    read_matrix(&mat, n, file);
    fclose(file);

    file= fopen(fileName2, "r");

    fgets(line, 256, file);
    sscanf (line, "%d", &n);

    read_matrix(&mat2, n, file);
    fclose(file);


    file = fopen(fileName3, "w");
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
    }


    for(int i =0; i < n; i++)
        free(mat[i]);
    free(mat);

    for(int i =0; i < n; i++)
        free(mat2[i]);
    free(mat2);

    /*clock_t end = clock();
    double time_spent = (double)(end - start); //in microseconds
    printf("Took %f micro-seconds \n", time_spent);*/

    return 0;
}

