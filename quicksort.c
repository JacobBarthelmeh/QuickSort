/**
  * Messing with quicksort
  * @author Jacob Barthelmeh
  */

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define NUMBER_ITT  1       /* k value in experiment */
#define MAX_ARRAY   10000  /* max array size to go to in experiment */
#define MULTIPLIER  1        /* value to multiple array size by */
#define ADDITION    1000   /* value to add to array size after an itt */

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

void partition_helper(int a, int b)
{
    int dif = (b-a) + 1;
    int ofs = a + (dif / 2);

    if (a == b || b > threadInfo.numberPartitions)
        return;
    
    //printf("partions %d and %d : a = %d\tb = %d\n",a,ofs,a,b);
    partition(threadInfo.p[a].head, threadInfo.p[a].size, &threadInfo.p[a], 
              &threadInfo.p[ofs]); 

    if ((b-a) <= 1)
        return;

    if (ofs > 1) {
        partition_helper(ofs, b);
        partition_helper(a, ofs - 1);
    }
}


/* Quick sort operation */
int qs(int* head, int sz)
{
    int  p; /* pivot point */
    int  v; /* pivot value */
    int* e; /* end point */
    int* h; /* current head of array */
    int  j; /* for loop index */
    int  temp;
    int  Ie;
    int  Ih;

    /* case of only one element in array */
    if (sz <= 1) {
        return 0;
    }

    if (sz <= partitionSz) {
        /* perform insertion sort */
        for (Ie = 0; Ie < sz - 1; Ie++) {
            v = INT_MAX; /* max integer value */
            p = 0;
            for (j = Ie; j < sz; j++) {
                if (head[j] < v) {
                    p = j;
                    v = head[j];
                }
            }
            temp = head[p];
            head[p] = head[Ie];
            head[Ie] = temp;
        }
    }
    else {
        /* perform quick sort */
        p = sz / 2;
        v = head[p];
        e = head + (sz - 1);
        h = head;
    
        /* find the sort */
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
        
        if (p <= 0) {
            qs(head, p + 1);
            qs(head + p + 1, sz - p - 1);
        }
        else {
            qs(head, p);
            qs(head + p, sz - p);
        }
    }

    return 0;
}


void* operation(void* in)
{
    struct info* data = (struct info*)in;

    /* case of only one element in array */
    if (data == NULL || data->size <= 1) {
        if (data->free)
            free(data);
        return NULL;
    }

    qs(data->head, data->size);

    if (data->free)
        free(data);

    return NULL;
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
    partition_helper(0, threadInfo.numberPartitions - 1);

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
        if (!threadInfo.t[j])
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

    end = clock();

    /* uncomment to have the array checked */
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

