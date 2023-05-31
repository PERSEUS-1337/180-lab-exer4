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
    int port;
    int chunk;
} MasterArgs;

float **createMatx(int rows, int cols)
{
    float **matrix = malloc(rows * sizeof(float *));

    for (int i = 0; i < rows; i++)
    {
        matrix[i] = malloc(cols * sizeof(float));
    }

    return matrix;
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

void printMatx(float **matx, int rows, int cols)
{
    printf("\nPrint the %d x %d (+1) Matrix:\n", cols - 1, rows - 1);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%.3f ", matx[i][j]);
        }
        printf("\n");
    }
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

// SLAVE: Receive MASTER messages
void handle_client(int client_socket)
{
    int recv_cols;
    int recv_rows;
    int bytesReceived1 = read(client_socket, &recv_cols, sizeof(int));
    int bytesReceived2 = read(client_socket, &recv_rows, sizeof(int));
    printf("\nReceived n: %d\n", recv_cols);
    printf("\nReceived size: %d\n", recv_rows);

    float **result = createMatx(recv_rows, recv_cols);
    int count = 0;
    recv_cols;
    recv_rows;
    // while (count < (recv_rows * recv_cols))
    // while (count < 130)
    // {
    //     printf("Count: %d\n", count);

    //     float recv_float;
    //     // Read the float
    //     int bytesReceived = read(client_socket, &recv_float, sizeof(float));
    //     // if (bytesReceived == sizeof(float))
    //     // {
    //     //     // Calculate the current position in the array
    //     //     int row = count / recv_cols;
    //     //     int col = count % recv_cols;

    //     //     printf("Row: %d, Col: %d\n", row, col);
    //     //     // Store the received element in the array
    //     //     result[row][col] = recv_float;

    //     //     // Increment the counter
    //     //     count++;
    //     // }
    //     // else
    //     // {
    //     //     // Error handling for read failure
    //     //     perror("Error receiving element");
    //     //     break;
    //     // }

    //     // int row = count / recv_cols;
    //     // int col = count % recv_cols;
    //     // printf("Row: %d, Col: %d\n", row, col);
    //     printf("Float: %f\n", recv_float);

    //     // Clear the buffer
    //     memset(&recv_float, 0, sizeof(recv_float));
    //     count++;
    // }
    for (int row = 0; row < recv_rows; row++)
    {
        for (int col = 0; col < recv_cols; col++)
        {
            float recv_float;
            // Read the float
            int bytesReceived = read(client_socket, &recv_float, sizeof(float));
            // printf("Float: %f\n", recv_float);

            // printf("Row: %d, Col: %d\n", row, col);
            result[row][col] = recv_float;
            memset(&recv_float, 0, sizeof(recv_float));
        }
    }
    printf("Last Element: %f\n", result[recv_rows-1][recv_cols-1]);
    // printMatx(result, recv_rows, recv_cols);

    // Close the client socket
    close(client_socket);
}

// SLAVE: Start listening on PORT
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

    printf("\nServer listening on port %d\n", port);

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
        printf("\nClient connected on port %d\n", port);
        // Handle the client connection in a separate function or thread
        handle_client(client_socket);
    }

    printf("\nServer listening on port %d has been closed.\n", port);

    // Close the server socket
    close(server_fd);

    return NULL;
}

// MASTER: Connect to SLAVE PORTS
void *conn_to_server(void *arg)
{
    MasterArgs *args = (MasterArgs *)arg;
    int port = args->port;
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

    send(client_fd, &args->n, sizeof(int), 0);
    send(client_fd, &args->chunk, sizeof(int), 0);

    for (int i = args->start; i < args->end; i++)
    {
        for (int j = 0; j < args->n; j++)
            send(client_fd, &args->M[i][j], sizeof(float), 0);
    }
    printf("\nMatrix sent!\n");

    // destroyMatx(matx, n);

    // // Clear the buffer
    // memset(matx, 0, sizeof(matx));
    // memset(array_recv, 0, sizeof(array_recv));

    // Close the connected socket
    close(client_fd);
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
        int port;

        printf("Enter your port: ");
        scanf("%d", &port);
        start_server(port);

        printf("Slave mode ending.\n");
    }
    else if (choice == 2)
    {
        int port;
        int num_slaves;
        int n = 0;
        srand(time(NULL));

        printf("Enter PORT: ");
        scanf("%d", &port);

        printf("Enter the number of slaves to connect to: ");
        scanf("%d", &num_slaves);

        printf("Enter n: ");
        int check = scanf("%d", &n);
        while (!check || n <= 9 || n % 10 != 0)
        {
            printf("Wrong Input! Try again: ");
            check = scanf("%d", &n);
        }

        // To compensate for extra 0-th coordinate
        n++;

        pthread_t threads[num_slaves];
        MasterArgs args[num_slaves];

        float **matx = createMatx(n, n);
        populateMatx(matx, n);

        // printMatx(matx, n, n);
        printf("\nLast Element: %f\n", matx[n-1][n-1]);

        int chunk_size = (n - 1) / num_slaves;

        for (int i = 0; i < num_slaves; i++)
        {
            printf("\nPort: %d\n", port);
            int min = getMin(i);
            int max = getMax(i);

            args[i].M = matx;
            args[i].n = n;

            if (i == 0)
                args[i].start = 0;
            else
                args[i].start = getMin((i)*chunk_size);

            args[i].end = getMax((i + 1) * chunk_size) + 1;
            args[i].port = port;
            args[i].chunk = args[i].end - args[i].start;

            printf("Start: %d, End: %d\n", args[i].start, args[i].end);

            pthread_create(&threads[i], NULL, conn_to_server, &args[i]);

            printf("Port Sent:%d\n", port);

            port++;
        }

        for (int i = 0; i < num_slaves; i++)
            pthread_join(threads[i], NULL);

        printf("\nMaster mode ending.\n");
    }
    else
    {
        printf("Invalid choice.\n");
        return 1;
    }

    return 0;
}