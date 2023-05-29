#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sched.h>

#define NUM_OF_CLIENTS 2

/**
 * Connects to the server at the specified port and sends a message.
 *
 * @param port The port number to connect to.
 * @param message The message to send to the server.
 */
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

int main(int argc, char const *argv[])
{
    int port;
    printf("Enter PORT: ");
    int check = scanf("%d", &port);
    connectToServer(port);

    // // Connect multiple clients to the server
    // for (int i = 0; i < NUM_OF_CLIENTS; i++)
    // {
    //     connectToServer(port + i);
    // }

    return 0;
}
