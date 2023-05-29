#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 6000
#define NUM_OF_CLIENTS 2
#define ARRAY_SIZE 5

// int running = 1;

void mult_array(const float *inputArray, float *outputArray, int size)
{
    for (int i = 0; i < size; i++)
    {
        outputArray[i] = inputArray[i] * 2.0; // Multiply each element by 2
    }
}

// Handle client connection
// Receive Client Messages, in this case, the array from client (MASTER)
void handle_client(int client_socket)
{
    int loop = 5;
    while (loop)
    {
        // const char *welcomeMsg = "Welcome to the server!";
        // send(client_socket, welcomeMsg, strlen(welcomeMsg), 0);

        // float array_send[ARRAY_SIZE] = {1.0, 69.69, 420.69, 1337.222, 777};

        // printf("Enter %d float numbers:\n", ARRAY_SIZE);
        // for (int i = 0; i < ARRAY_SIZE; i++)
        // {
        //     printf("Enter number %d: ", i + 1);
        //     scanf("%f", &array[i]);
        // }


        // Read client message
        // char buffer[1024] = {0};
        // int valread;
        // valread = read(client_socket, buffer, 1024);
        // if (valread <= 0)
        // {
        //     // Error or connection closed by client
        //     // running = 0;
        //     break;
        // }

        // printf("Received message: %s\n", buffer);

        // // Check for termination message
        // if (strcmp(buffer, "quit") == 0)
        // {
        //     printf("Termination message received. Closing connection.\n");
        //     // running = 0;
        //     break;
        // }

        float array_recv[ARRAY_SIZE];
        float array_send[ARRAY_SIZE];
        // Read the array
        int bytesReceived = read(client_socket, array_recv, sizeof(array_recv));
        // Calculate the number of elements in the received array
        int numElements = bytesReceived / sizeof(float);

        // Print the received array
        printf("Received array: ");
        for (int i = 0; i < numElements; i++)
        {
            printf("%.2f ", array_recv[i]);
        }
        printf("\n");

        mult_array(array_recv, array_send, ARRAY_SIZE);

        printf("Sent modified array: ");
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            printf("%.2f ", array_send[i]);
        }
        printf("\n");

        send(client_socket, array_send, sizeof(array_send), 0);
        // printf("Array sent\n");

        // Clear the buffer
        memset(array_recv, 0, sizeof(array_recv));
        memset(array_send, 0, sizeof(array_send));
        loop--;
    }

    // Close the client socket
    close(client_socket);
}

void *start_server(void *arg)
{
    int port = *(int *)arg;
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

void conn_to_server(int port)
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    // Set up the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    // Connect to the server
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    while (1)
    {
        // Send the message to the server
        float array_send[ARRAY_SIZE] = {1.0, 69.69, 420.69, 1337.222, 777};

        printf("Sent float array: ");
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            printf("%.2f ", array_send[i]);
        }
        printf("\n");

        send(client_fd, array_send, sizeof(array_send), 0);
        // printf("Array sent\n");

        float array_recv[ARRAY_SIZE];
        // Read the array
        int bytesReceived = read(client_fd, array_recv, sizeof(array_recv));
        // Calculate the number of elements in the received array
        int numElements = bytesReceived / sizeof(float);

        // Print the received array
        printf("Received array: ");
        for (int i = 0; i < numElements; i++)
        {
            printf("%.2f ", array_recv[i]);
        }
        printf("\n");

        // // Clear the buffer
        memset(array_send, 0, sizeof(array_send));
        memset(array_recv, 0, sizeof(array_recv));

        // Receive and print the server's response
        // valread = read(client_fd, buffer, 1024);
        // printf("Server response: %s\n", buffer);
    }

    // Close the connected socket
    close(client_fd);
}

int main()
{
    int choice;
    printf("Choose mode:\n");
    printf("1. Server\n");
    printf("2. Client\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        int numThreads;
        printf("Enter the number of threads to create: ");
        scanf("%d", &numThreads);

        pthread_t threads[numThreads];
        int ports[numThreads];

        for (int i = 0; i < numThreads; i++)
        {
            ports[i] = PORT + i;

            if (pthread_create(&threads[i], NULL, start_server, (void *)&ports[i]) != 0)
            {
                perror("Error creating thread");
                exit(1);
            }
        }

        for (int i = 0; i < numThreads; i++)
        {
            pthread_join(threads[i], NULL);
        }

        printf("Server mode ending.\n");
    }
    else if (choice == 2)
    {
        int port;
        printf("Enter PORT: ");
        scanf("%d", &port);
        conn_to_server(port);

        printf("Client mode ending.\n");
    }
    else
    {
        printf("Invalid choice.\n");
        return 1;
    }

    return 0;
}
