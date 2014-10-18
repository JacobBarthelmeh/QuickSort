/**
  * Messing with quicksort
  * @author Jacob Barthelmeh
  */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define NUMBER_ITT  10       /* k value in experiment */
#define MAX_ARRAY   1000000  /* max array size to go to in experiment */
#define MULTIPLIER  1        /* value to multiple array size by */
#define ADDITION    100000   /* value to add to array size after an itt */

static unsigned int partitionSz;

struct info {
    int  size;
    int  free;
    int* head;
};

struct threads {
    pthread_t*   t;
    struct info* p;
    int numberPartitions;
    int numberThreads;
    int max;
};
static struct threads threadInfo;


/* create partitions of the data */
int partition(int* data, int sz,  struct info* in1, struct info* in2)
{
    int  p; /* pivot point */
    int  v; /* pivot value */
    int* e; /* end point */
    int* h; /* current head of array */
    int  j; /* for loop index */
    int Ie;
    int Ih;

    if (sz <= 1) {
        in2->size = 0;
        in2->free = 0;
        in2->head = NULL;
        return 0;
    }

    if (in1 == NULL || in2 == NULL) {
            printf("Error invalid argument to partition!\n");
            exit(1);
    }
        
    p = sz / 2;
    v = data[p];
    e = data + (sz - 1);
    h = data;

    /* find the sort and store in the temp array */
    j = 0;
    Ih = 0;
    Ie = sz - 1;
    while (Ih < Ie) {
        if (*h > v && Ie >= p) {
            while (*e > v && Ie >= p) {e--; Ie--;}
            if (Ie == p)
                p = Ih;
            /* swap */
            j = *e;
            *e = *h;
            *h = j;
        }

        if (*e < v && Ih <= p) {
            while (*h < v && Ih <= p) {h++; Ih++;}
            if (Ih == p)
                p = Ie;
            /* swap */
            j = *e;
            *e = *h;
            *h = j;
        }

        if (Ih < p) {
            h++;
            Ih++;
        }
        if (Ie > p) {
            e--;
            Ie--;
        }
    }

    in1->size = p;
    in1->head = data;
    in2->size = sz - p;
    in2->head = data + p;

    if (p <= 0) {
        in2->head++;
        in2->size--;
        in1->size++;
    }
    
    in1->free = 0;
    in2->free = 0;

    return 0;
}


void* operation(void* in)
{
    struct info* data = (struct info*)in;
    struct info* dataTemp1;
    struct info* dataTemp2;
    int  p; /* pivot point */
    int  v; /* pivot value */
    int* e; /* end point */
    int* h; /* current head of array */
    int  j; /* for loop index */
    int  temp;
    int  Ie;
    int  Ih;

    /* case of only one element in array */
    if (data == NULL || data->size <= 1) {
        if (data->free)
            free(data);
        return NULL;
    }

    if (data->size <= partitionSz) {
        /* perform insertion sort */
        for (Ie = 0; Ie < data->size - 1; Ie++) {
            v = INT_MAX; /* max integer value */
            p = 0;
            for (j = Ie; j < data->size; j++) {
                if (data->head[j] < v) {
                    p = j;
                    v = data->head[j];
                }
            }
            temp = data->head[p];
            data->head[p] = data->head[Ie];
            data->head[Ie] = temp;
        }
    }
    else {
        /* perform quick sort */
        dataTemp1 = malloc(sizeof(struct info));
        dataTemp2 = malloc(sizeof(struct info));
        if (dataTemp1 == NULL || dataTemp2 == NULL) {
            printf("Error in allocating memory!\n");
            exit(1);
        }
        
        dataTemp1->free = 1;
        dataTemp2->free = 1;

        p = data->size / 2;
        v = data->head[p];
        e = data->head + (data->size - 1);
        h = data->head;
    
        /* find the sort */
        j = 0;
        Ih = 0;
        Ie = data->size - 1;
        while (Ih < Ie) {
            if (*h > v && Ie >= p) {
                while (*e > v && Ie >= p) {e--; Ie--;}
                if (Ie == p)
                    p = Ih;
                /* swap */
                j = *e;
                *e = *h;
                *h = j;
            }
    
            if (*e < v && Ih <= p) {
                while (*h < v && Ih <= p) {h++; Ih++;}
                if (Ih == p)
                    p = Ie;
                /* swap */
                j = *e;
                *e = *h;
                *h = j;
            }
    
            if (Ih < p) {
                h++;
                Ih++;
            }
            if (Ie > p) {
                e--;
                Ie--;
            }
        }
        
        dataTemp1->size = p;
        dataTemp1->head = data->head;
        dataTemp2->size = data->size - p;
        dataTemp2->head = data->head + p;

        if (p <= 0) {
            dataTemp2->head++;
            dataTemp2->size--;
            dataTemp1->size++;
        }

        operation((void*)dataTemp1);
        operation((void*)dataTemp2);
    }

    if (data->free)
        free(data);

    return NULL;
}


/**
  * Function to print out an array to the terminal
  */
int output(int* a, int sz)
{
    int* i;

    for (i = a; i < (a + sz); i++) {
        printf("%d\t", *i);
    }
    fflush(stdout);
    printf("\n");

    return 0;
}


/**
  * Checks if the array of ints is sorted
  */
int check(int* a, int sz)
{
    int i;

    /* check if correct */
    for (i = 0; i <  sz - 1; i++) {
        if (a[i] > a[i + 1]) {
            printf("\n**Failes**\n");
            return 1;
        }
    }

    printf("\n**Sorted**\n");

    return 0;
}


double run_test(int arrSz, int parSz, int numThreads, int seed)
{
    int* a; /* array of random numbers */
    int* i;
    int  j;
    int swap = 0;
    struct info data;
    clock_t begin, end;

    /* case of one element or smaller array */
    if (arrSz <= 1) {
        return 0.0;
    }

    a = malloc(sizeof(int) * arrSz);
    if (a == NULL) {
        printf("Not enough memory to allocate array!\n");
        return -1.0;
    }
    
    /* set up structure for threads */
    threadInfo.numberThreads = numThreads;
    threadInfo.max = numThreads;
    if (threadInfo.numberThreads < 1)
        threadInfo.numberThreads = 1;
    threadInfo.t = malloc(threadInfo.numberThreads * sizeof(pthread_t));

    /* create an array of random numbers */
    srand(seed);
    for (i = a; i < (a + arrSz); i++) {
        *i = rand();
    }

    /* start the sorting process */
    data.size = arrSz;
    data.head = a;
    data.free = 0;

    begin = clock();
    /* create partitions */
    for (threadInfo.numberPartitions = 1; threadInfo.numberPartitions <
                  threadInfo.numberThreads; threadInfo.numberPartitions *= 2) {}
    threadInfo.p = malloc(threadInfo.numberPartitions * sizeof(struct info));

    threadInfo.p[0] = data;
    for (j = 1; j < threadInfo.numberPartitions; j = j * 2) {
        int e;
        for (e = 0; e < j; e++) {
            partition(threadInfo.p[e].head, threadInfo.p[e].size,
                      &threadInfo.p[e], &threadInfo.p[e + j]);
            //output(threadInfo.p[e].head, threadInfo.p[e].size);
            //output(threadInfo.p[e+j].head, threadInfo.p[e+j].size);
        }
        if (swap > 0) {
            struct info tempData;
            for (e = 1; e < j; e++) {
                tempData = threadInfo.p[e];
                threadInfo.p[e] = threadInfo.p[j-1+e];
                threadInfo.p[j-1+e] = tempData;
            }        
        } 
        swap++;
    }

    /* malloc space for each thread to check if cahing is taking a hit */
    i = a;
    for (j = 0; j < threadInfo.numberThreads; j++) {
        threadInfo.p[j].head = malloc(threadInfo.p[j].size * sizeof(int));
        for (int e = 0; e < threadInfo.p[j].size; e++) {
            threadInfo.p[j].head[e] = *i;
            i++;
        }
    }

    /* create threads */
    for (j = 0; threadInfo.numberThreads > 0; threadInfo.numberThreads--) {
        //printf("created a new thread\n");
        pthread_create(&threadInfo.t[j], NULL,
                   operation, (void*) &threadInfo.p[j]);
        j++;
    }

    swap = threadInfo.max;
chopper:
    for (j = 0; j < threadInfo.max; j++) {
        if (threadInfo.t[j] == NULL)
            printf("thread was null\n");
        pthread_join(threadInfo.t[j], NULL);
        /* case when there is more partitions than threads */
        if (swap < threadInfo.numberPartitions) {
            pthread_create(&threadInfo.t[j], NULL,
                   operation, (void*) &threadInfo.p[swap]);
            swap++;
        }
    }

    /* case if there still is partitions left over, back to the chopper */
    if (swap < threadInfo.numberPartitions)
        goto chopper;

    /* go through join once more to make sure all threads are joined */
    if (swap == threadInfo.numberPartitions) {
        swap++;
        goto chopper;
    }

    /* Put values back into main array */
    i = a;
    for (j = 0; j < threadInfo.max; j++) {
        for (int e = 0; e < threadInfo.p[j].size; e++) {
            *i = threadInfo.p[j].head[e];
            i++;
        }
        free(threadInfo.p[j].head);
    }
    end = clock();

    //check(a, arrSz);

    /* clean up */
    free(threadInfo.t);
    free(threadInfo.p);
    free(a);

    return ((double)(end - begin)/CLOCKS_PER_SEC);
}


int main(int argc, char* argv[])
{
    int  i;
    int sz;
    int seed, curSeed;
    int numT;
    double  average;
    double* times;
    FILE*   output;
    FILE*   avgF;

    if (argc != 5) {
        printf("Call program with size of array, partition size, number of threads and seed value as arguments"
               "\n./<program> <size-array> <size-partition> <number of threads> <seed>\n");
        return 1;
    }

    sz          = atoi(argv[1]);
    partitionSz = atoi(argv[2]);
    numT        = atoi(argv[3]);
    seed        = atoi(argv[4]);
    output      = fopen("output.csv", "w");
    avgF        = fopen("average.csv", "w");
    if (output == NULL || avgF == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    fputs("Array Size,Time\n", output);
    fputs("Array Size,Time\n", avgF);
    while (sz <= MAX_ARRAY) {
        curSeed = seed;
        average = 0;
	    times = malloc(NUMBER_ITT * sizeof(double));

	    for (i = 0; i < NUMBER_ITT; i++) {
	        times[i] = run_test(sz, partitionSz, numT, curSeed++);
	    }
	
	    for (i = 0; i < NUMBER_ITT; i++) {
            fprintf(output, "%d,%f\n", sz, times[i]);
            average += times[i];
	    }
        average = average / ((double)NUMBER_ITT);
        fprintf(avgF, "%d,%f\n", sz, average);
	
	    free(times);
        printf("....done with array size %d : average time of %f\n", sz, average);
        sz = sz * MULTIPLIER;
        sz = sz + ADDITION;
    }

    fclose(output);
    fclose(avgF);

    return 0;
}

