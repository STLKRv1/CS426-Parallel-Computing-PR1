
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[])
{
    //clock_t start = clock();

    char* fileName;
    if(argc>=2)
    {
        fileName = argv[1];
    }

    FILE* file = fopen(fileName, "r");
    char line[256];
    int sum = 0;
    int num = 0;

    while ( fgets(line, 256, file)!=NULL) {
        sscanf (line, "%d", &num);
        //printf("%s", line);
        sum += num;
    }

    printf("sum: %d\n", sum);
    fclose(file);

    /*clock_t end = clock();
    double time_spent = (double)(end - start); //in microseconds
    printf("Took %f micro-seconds \n", time_spent);*/

    return 0;
}

