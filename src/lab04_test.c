#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

#define PORT 6000
#define NUM_OF_CLIENTS 2
#define ARRAY_SIZE 5
#define NUM_CORES 12

typedef struct
{
    float **M;
    int n;
    int start;
    int end;
    int core_id;
} ThreadArgs;

void mult_array(const float *inputArray, float *outputArray, int size)
{
    for (int i = 0; i < size; i++)
    {
        outputArray[i] = inputArray[i] * 2.0; // Multiply each element by 2
    }
}

float **createMatx(int n)
{
    float **rows = malloc(n * sizeof(float *));

    for (int i = 0; i < n; i++)
        rows[i] = malloc(sizeof(float) * n);

    return rows;
}

void destroyMatx(float **matx, int n)
{
    for (int i = 0; i < n; i++)
        free(matx[i]);

    free(matx);

    matx = NULL;
}

void populateMatx(float **matx, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if ((i % 10 == 0) && (j % 10 == 0))
                matx[i][j] = (rand() % (1000));
}

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

// Handle client connection
// Receive Client Messages, in this case, the array from client (MASTER)
void handle_client(int client_socket)
{
    while (1)
    {
        float array_recv[11];
        // float array_send[ARRAY_SIZE];

        // Read the array
        int bytesReceived = read(client_socket, array_recv, sizeof(array_recv));
        if (bytesReceived <= 0)
        {
            // Error or connection closed by client
            break;
        }
        // Calculate the number of elements in the received array
        int numElements = bytesReceived / sizeof(float);

        // Print the received array
        printf("Received array: ");
        for (int i = 0; i < numElements; i++)
        {
            printf("%.2f ", array_recv[i]);
        }
        printf("\n");

        // mult_array(array_recv, array_send, ARRAY_SIZE);

        // printf("Sent modified array: ");
        // for (int i = 0; i < ARRAY_SIZE; i++)
        // {
        //     printf("%.2f ", array_send[i]);
        // }
        // printf("\n");

        // send(client_socket, array_send, sizeof(array_send), 0);
        // printf("Array sent\n");

        // Clear the buffer
        memset(array_recv, 0, sizeof(array_recv));
        // memset(array_send, 0, sizeof(array_send));
    }

    // Close the client socket
    close(client_socket);
}

// void *start_server(void *arg)
void *start_server(int port)
{
    // int port = *(int *)arg;
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    // Bind the server socket to the specified port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Error listening");
        exit(1);
    }

    printf("Server listening on port %d\n", port);

    while (1)
    {
        // Accept a client connection
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            exit(1);
        }
        printf("Client connected on port %d\n", port);
        // Handle the client connection in a separate function or thread
        handle_client(client_socket);
    }

    printf("Server listening on port %d has been closed.\n", port);

    // Close the server socket
    close(server_fd);

    return NULL;
}

void *conn_to_server(void *arg)
{
    int port = *(int *)arg;
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return NULL;
    }

    // Set up the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }

    // Connect to the server
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return NULL;
    }

    // while (1)
    // {
        // // Send the message to the server
        // float array_send[ARRAY_SIZE] = {1.0, 69.69, 420.69, 1337.222, 777};

        // printf("Sent float array: ");
        // for (int i = 0; i < ARRAY_SIZE; i++)
        // {
        //     printf("%.2f ", array_send[i]);
        // }
        // printf("\n");

        // send(client_fd, array_send, sizeof(array_send), 0);
        // // printf("Array sent\n");

        // float array_recv[ARRAY_SIZE];
        // // Read the array
        // int bytesReceived = read(client_fd, array_recv, sizeof(array_recv));
        // // Calculate the number of elements in the received array
        // int numElements = bytesReceived / sizeof(float);

        // // Print the received array
        // printf("Received array: ");
        // for (int i = 0; i < numElements; i++)
        // {
        //     printf("%.2f ", array_recv[i]);
        // }
        // printf("\n");

        int n = 10;
        // printf("Enter n divisible by 10: ");
        // int check = scanf("%d", &n);

        // while (!check || n <= 9 || n % 10 != 0)
        // {
        //     printf("Wrong Input! Try again: ");
        //     check = scanf("%d", &n);
        // }
        n++;

        float **matx = createMatx(n);
        populateMatx(matx, n);

        for (int i = 0; i < n; i++)
        {
            send(client_fd, matx[i], n * sizeof(float), 0);
        }
        printMatx(matx, n);
        printf("Matrix sent!\n");

        // destroyMatx(matx, n);
        
        // // Clear the buffer
        memset(matx, 0, sizeof(matx));
        // memset(array_recv, 0, sizeof(array_recv));
    // }

    // Close the connected socket
    close(client_fd);
}

void terrain_inter(float **M, int n, int num_threads, int port)
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
        // pthread_create(&threads[i], NULL, thread_func, &args[i]);
        pthread_create(&threads[i], NULL, conn_to_server, &port);
    }

    // This will join the threads if they are finished in their operation, therefore finishing this terrain_inter function
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main()
{
    int choice;
    printf("Choose mode:\n");
    printf("1. Slave\n");
    printf("2. Master\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        // int numThreads;
        // printf("Enter the number of threads to create: ");
        // scanf("%d", &numThreads);

        // pthread_t threads[numThreads];
        // // int ports[numThreads];
        int port;

        // for (int i = 0; i < numThreads; i++)
        // {
        //     // ports[i] = PORT + i;
            // printf("Enter your port: ");
            // scanf("%d", &port);
        printf("Enter your port: ");
        scanf("%d", &port);
        start_server(port);

        //     if (pthread_create(&threads[i], NULL, start_server, (void *)&port) != 0)
        //     {
        //         perror("Error creating thread");
        //         exit(1);
        //     }
        // }

        // for (int i = 0; i < numThreads; i++)
        // {
        //     pthread_join(threads[i], NULL);
        // }

        printf("Slave mode ending.\n");
    }
    else if (choice == 2)
    {
        int port;
        printf("Enter PORT: ");
        scanf("%d", &port);

        int num_slaves;
        printf("Enter the number of slaves to connect to: ");
        scanf("%d", &num_slaves);

        pthread_t threads[num_slaves];

        int n = 10;
        n++
        float **matx = createMatx(n);
        populateMatx(matx, n);

        for (int i = 0; i < num_slaves; i++)
        {
            port+=i;
            terrain_inter(matx, n, num_slaves, port);
            // pthread_create(&threads[i], NULL, conn_to_server, &port);
        }
            // port++;
            // conn_to_server(port + i);

        for (int i = 0; i < num_slaves; i++)
            pthread_join(threads[i], NULL);

        printf("Master mode ending.\n");
    }
    else
    {
        printf("Invalid choice.\n");
        return 1;
    }

    return 0;
}
