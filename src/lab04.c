// CMSC 180 Lab 04
// Author: Aron Resty Ramillano | 2020-01721
// Section: T3L

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
// For sending data through sockets efficiently

#include <msgpack.h>

#define NUM_CORES 12

typedef struct
{
    float **M;
    int n;
    int start;
    int end;
    int core_id;
} ThreadArgs;

/**
 * It creates a matrix of size n x n
 *
 * @param n the number of rows and columns in the matrix
 *
 * @return A pointer to a pointer to a float.
 */
float **createMatx(int n)
{
    float *values = malloc(n * sizeof(float));
    float **rows = malloc(n * sizeof(float *));
    for (int i = 0; i < n; ++i)
    {
        rows[i] = malloc(sizeof(float) * n);
    }
    return rows;
}

/**
 * It frees the memory allocated for the matrix.
 *
 * @param matx The matrix to be destroyed.
 */
void destroyMatx(float **matx, int n)
{
    // Old implementation
    // free(*matx);
    // free(matx);

    // New Implementation
    // Free mem of each row
    for (int i = 0; i < n; ++i)
    {
        free(matx[i]);
    }
    // Free mem of array of row pointers
    free(matx);
}

int getMin(int n)
{
    int min = 0;

    if (n > 1)
        min = floor((n - 1) / 10) * 10;

    return min;
}

int getMax(int n)
{
    int max = 10;

    if (n > 0)
        max = ceil(n / 10.0) * 10;

    return max;
}

/**
 * It prints a matrix of size n x n
 *
 * @param matx The matrix to be printed
 * @param n the number of rows and columns in the matrix
 */
void printMatx(float **matx, int n)
{
    printf("\nPrint the %d x %d Matrix:\n", n - 1, n - 1);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%.3f ", matx[i][j]);
        }
        printf("\n");
    }
}

/**
 * This function populates the matrix with random numbers. It will only populate on coordinates that is divisible by 10, including 0,0
 *
 * @param matx the matrix to be populated
 * @param n the size of the matrix
 */
void populateMatx(float **matx, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if ((i % 10 == 0) && (j % 10 == 0))
                matx[i][j] = (rand() % (1000));
}

/**
 * It takes a 2D array of floats, and for each element in the array, it calculates the average
 * elevation of the surrounding elements
 *
 * @param arg The argument passed to the thread function.
 *
 * @return a void pointer.
 */
void *thread_func(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    const pthread_t pid = pthread_self();
    const int core_id = args->core_id;

    // cpu_set_t: This data set is a bitset where each bit represents a CPU.
    cpu_set_t cpuset;
    // CPU_ZERO: This macro initializes the CPU set to be the empty set.
    CPU_ZERO(&cpuset);
    // CPU_SET: This macro adds cpu to the CPU set.
    CPU_SET(core_id, &cpuset);

    const int set_result = pthread_setaffinity_np(pid, sizeof(cpu_set_t), &cpuset);

    // Comment out code below to see results
    // const int get_affinity = pthread_getaffinity_np(pid, sizeof(cpu_set_t), &cpuset);
    // printf("core_id: %d, set_result: %d, get_affinity: %d\n", core_id, set_result, get_affinity);

    for (int i = args->start; i < args->end; i++)
    {
        int min_x = getMin(i);
        int max_x = getMax(i);
        for (int j = 0; j < args->n; j++)
        {
            if (!((i % 10 == 0) && (j % 10 == 0)))
            {
                int min_y = getMin(j);
                int max_y = getMax(j);

                int area_d = (abs(min_x - i) * abs(min_y - j));
                int area_c = (abs(max_x - i) * abs(min_y - j));
                int area_b = (abs(min_x - i) * abs(max_y - j));
                int area_a = (abs(max_x - i) * abs(max_y - j));

                float elev_a = args->M[min_x][min_y];
                float elev_b = args->M[max_x][min_y];
                float elev_c = args->M[min_x][max_y];
                float elev_d = args->M[max_x][max_y];

                float elevation = ((area_a * elev_a) + (area_b * elev_b) + (area_c * elev_c) + (area_d * elev_d)) / (float)(area_a + area_b + area_c + area_d);

                args->M[i][j] = elevation;
                // printf("Boundary: %i %i | Coords: %i %i | Val: %f\n", args->start, args->end, i, j, elevation);
            }
        }
    }

    return NULL;
}

/**
 * It creates a thread for each submatrix that we are going to create
 *
 * @param M The matrix that we are going to be iterating through
 * @param n The size of the matrix
 * @param num_threads The number of threads that will be created
 */
void terrain_inter(float **M, int n, int num_threads)
{
    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    // This is basically the submatrices that we are going to make.
    int chunk_size = n / num_threads;

    for (int i = 0; i < num_threads; i++)
    {
        args[i].M = M;
        args[i].n = n;
        args[i].start = i * chunk_size;
        args[i].end = (i + 1) * chunk_size;
        args[i].core_id = (i % NUM_CORES);

        // Because the matrix should include the 0th coordinate, we would have to adjust our calculation to have the last submatrix to be able to handle a second row.
        if (num_threads > 1 && i == num_threads - 1)
        {
            args[i].end++;
        }

        // Actual implementation of creating a thread
        // We pass the arguments stated above on where on the matrix they should start
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    // This will join the threads if they are finished in their operation, therefore finishing this terrain_inter function
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

/**
 * It creates a matrix of size n, populates it with random values, and then calls the terrain_inter
 * function.
 * This benchmark is designed to output the necessary information needed for the exercise lab report
 *
 * @param n The size of the matrix.
 */
void run_benchmark(int n, int t)
{
    // Generate new RNG
    srand(time(NULL));
    n++;

    float **matx = createMatx(n);
    populateMatx(matx, n);

    // Get start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Do interpolation
    terrain_inter(matx, n, t);

    // Get end time
    gettimeofday(&end_time, NULL);
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long micros = ((seconds * 1000000) + end_time.tv_usec) - (start_time.tv_usec);
    double elapsed_time = (double)micros / 1000000.0;

    // Clean matrix print
    // printMatx(matx, n);

    printf("Threads: %i, Elapsed time: %.5f seconds\n", t, elapsed_time);

    // Free memory
    destroyMatx(matx, n);
}

void benchmark_input()
{
    // Matrix size input
    int n = 0, sleep_sec = 0;
    printf("Enter n (for benchmark): ");
    int check = scanf("%d", &n);

    /*
    Input checker on:
    - if no input
    - if n <= 9
    - if n not divisible by 10
    */
    while (!check || n <= 9 || n % 10 != 0)
    {
        printf("Wrong Input! Try again: ");
        check = scanf("%d", &n);
    }

    printf("\nCalculating a %d x %d matrix\nBenchmark will run 3x\nSleep for %d seconds each run\n", n, n, sleep_sec);

    for (int i = 0; i < 3; i++)
    {
        printf("Run # %d\n", i + 1);
        for (int j = 1; j <= 64; j *= 2)
        {
            run_benchmark(n, j);
        }

        // Sleep after every run
        if (i == 2)
            break;
        if (sleep_sec)
        {
            printf("...Sleep for %d seconds...\n", sleep_sec);
            sleep(sleep_sec);
        }
    }
}

void user_input()
{
    // Matrix size input
    int n = 0, t = 0;
    printf("Enter n divisible by 10: ");
    int check = scanf("%d", &n);

    /*
    Input checker on:
    - if no input
    - if n <= 9
    - if n not divisible by 10
    */
    while (!check || n <= 9 || n % 10 != 0)
    {
        printf("Wrong Input! Try again: ");
        check = scanf("%d", &n);
    }

    // Thread number input
    printf("Enter t that can divide n: ");
    check = scanf("%d", &t);

    /*
    Input checker on:
    - if no input
    - if n is not divisible by t
    - if t < 1
    */
    while (!check || n % t != 0 || t < 1)
    {
        printf("Wrong Input! Try again: ");
        check = scanf("%d", &t);
    }

    // +1 to include 10th coord
    n++;

    // Generate new RNG
    srand(time(NULL));

    // Create and Populate Matrix
    float **matx = createMatx(n);
    populateMatx(matx, n);

    // Get start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Do interpolation
    terrain_inter(matx, n, t);

    // Get end time
    gettimeofday(&end_time, NULL);
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long micros = ((seconds * 1000000) + end_time.tv_usec) - (start_time.tv_usec);
    double elapsed_time = (double)micros / 1000000.0;

    // Clean matrix print
    // printMatx(matx, n);

    printf("\nElapsed time: %.5f seconds\n", elapsed_time);

    destroyMatx(matx, n);
}

void packer_test()
{
    int n = 0;
    printf("Enter n: ");
    int check = scanf("%d", &n);
    printf("n = %d\n", n);

    // Initialize a buffer for packing and unpacking
    size_t buffer_size = 1024;
    char buffer[buffer_size];

    // Create an integer to pack
    // int n = 42;

    // Pack the integer into the buffer
    printf("Initiallizing packer and buffer...\n");
    msgpack_sbuffer sbuffer;
    msgpack_sbuffer_init(&sbuffer);
    msgpack_packer packer;
    msgpack_packer_init(&packer, &sbuffer, msgpack_sbuffer_write);

    printf("Integer packed!\n");
    msgpack_pack_int(&packer, n);


    printf("Initiallizing unpacker...\n");
    // Unpack the integer from the buffer
    msgpack_unpacked unpacked;
    msgpack_unpacked_init(&unpacked);

    printf("Transferring unpacked data...\n");
    memcpy(buffer, sbuffer.data, sbuffer.size);

    printf("Unpacking the data...\n");
    // Unpack the data from the buffer
    msgpack_unpack_return result = msgpack_unpack_next(&unpacked, buffer, sbuffer.size, NULL);

    // Check if the unpacking was successful
    if (result == MSGPACK_UNPACK_SUCCESS)
    {
        // Check if the unpacked object is an integer
        if (unpacked.data.type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
            unpacked.data.type == MSGPACK_OBJECT_NEGATIVE_INTEGER)
        {
            int unpackedValue = (int)unpacked.data.via.i64;
            printf("Unpacked integer: %d\n", unpackedValue);
        }
        else
        {
            printf("Error: Unpacked object is not an integer\n");
        }
    }
    else
    {
        printf("Error: Failed to unpack data\n");
    }

    printf("Cleaning up packers and buffers...\n");
    // Clean up the resources
    msgpack_sbuffer_destroy(&sbuffer);
    msgpack_unpacked_destroy(&unpacked);
}

int main()
{
    // Matrix size input
    int n = 0;
    printf("Enter [1] for User Input\nEnter [2] for auto benchmark\nEnter [3] for data testing\n>>> ");
    int check = scanf("%d", &n);

    /*
    Input checker on:
    - if no input
    - if n <= 9
    - if n not divisible by 10
    */
    while (!check || n > 3 || n < 1)
    {
        printf("Wrong Input! Try again\n>>> ");
        check = scanf("%d", &n);
    }
    if (n == 1)
        user_input();
    else if (n == 2)
        benchmark_input();
    else
        packer_test();

    return 0;
}