#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sched.h>

#define PORT 5000

int running = 1;

void handleClient(int clientSocket)
{
    // Handle client connection
    // You can implement your own logic here
    // For demonstration purposes, we'll just send a welcome message

    while (running)
    {
        const char *welcomeMsg = "Welcome to the server!";
        send(clientSocket, welcomeMsg, strlen(welcomeMsg), 0);

        char buffer[1024] = {0};
        int valread;
        // Read client message
        valread = read(clientSocket, buffer, 1024);
        if (valread <= 0)
        {
            // Error or connection closed by client
            running = 0;
            break;
        }

        printf("Received message: %s\n", buffer);

        // Check for termination message
        if (strcmp(buffer, "quit") == 0)
        {
            printf("Termination message received. Closing connection.\n");
            running = 0;
            break;
        }

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));
    }

    // Close the client socket
    close(clientSocket);
}

void *startServer(void *arg)
{
    int port = *(int *)arg;
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    // Bind the server socket to the specified port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0)
    {
        perror("Error listening");
        exit(1);
    }

    printf("Server listening on port %d\n", port);

    while (running)
    {
        // Accept a client connection
        clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            perror("Error accepting connection");
            exit(1);
        }
        printf("Client connected on port %d\n", port);
        // Handle the client connection in a separate function or thread
        handleClient(clientSocket);
    }

    printf("Server listening on port %d has been closed.\n", port);

    // Close the server socket
    close(serverSocket);

    return NULL;
}

int main()
{
    // Create threads for each server
    pthread_t thread1, thread2;
    int port1 = PORT;
    int port2 = PORT + 1;

    if (pthread_create(&thread1, NULL, startServer, (void *)&port1) != 0)
    {
        perror("Error creating thread for port #1");
        exit(1);
    }

    if (pthread_create(&thread2, NULL, startServer, (void *)&port2) != 0)
    {
        perror("Error creating thread for port #2");
        exit(1);
    }

    // Wait for the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program ending.\n");

    return 0;
}
