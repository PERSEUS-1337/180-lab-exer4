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

// int running = 1;

// Handle client connection
void handleClient(int clientSocket)
{

    while (1)
    {
        const char *welcomeMsg = "Welcome to the server!";
        send(clientSocket, welcomeMsg, strlen(welcomeMsg), 0);

        // Read client message
        char buffer[1024] = {0};
        int valread;
        valread = read(clientSocket, buffer, 1024);
        if (valread <= 0)
        {
            // Error or connection closed by client
            // running = 0;
            break;
        }

        printf("Received message: %s\n", buffer);

        // Check for termination message
        if (strcmp(buffer, "quit") == 0)
        {
            printf("Termination message received. Closing connection.\n");
            // running = 0;
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

    while (1)
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

void connectToServer(int port)
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
        char message[1024];
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);
        // Remove newline character from the message
        message[strcspn(message, "\n")] = '\0';

        send(client_fd, message, strlen(message), 0);
        printf("Message sent from PORT %d: %s\n", port, message);

        // Check if the message is "quit"
        if (strcmp(message, "quit") == 0)
            break;

        // Receive and print the server's response
        valread = read(client_fd, buffer, 1024);
        printf("Server response: %s\n", buffer);
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
    printf("3. Local\n");

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

            if (pthread_create(&threads[i], NULL, startServer, (void *)&ports[i]) != 0)
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
        connectToServer(port);

        printf("Client mode ending.\n");
    }
    else if (choice == 3)
    {
        
    }
    else
    {
        printf("Invalid choice.\n");
        return 1;
    }

    return 0;
}
