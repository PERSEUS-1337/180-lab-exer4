#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NUM_PORTS 2
#define PORT1 8080
#define PORT2 8081

int main(int argc, char const *argv[])
{
    int server_fd[NUM_PORTS];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create and bind sockets for multiple ports
    for (int i = 0; i < NUM_PORTS; i++)
    {
        // Create socket file descriptor
        if ((server_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Set socket options and bind
        if (setsockopt(server_fd[i], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;

        // Bind to different ports based on the iteration
        if (i == 0)
        {
            address.sin_port = htons(PORT1);
        }
        else if (i == 1)
        {
            address.sin_port = htons(PORT2);
        }

        // Bind the socket to the port
        if (bind(server_fd[i], (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Listen for incoming connections
        if (listen(server_fd[i], 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    // Accept and handle incoming connections for each port
    while (1)
    {
        for (int i = 0; i < NUM_PORTS; i++)
        {
            int new_socket;
            if ((new_socket = accept(server_fd[i], (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Handle the connection for this port

            // Close the connected socket
            close(new_socket);
        }
    }

    // Close listening sockets
    for (int i = 0; i < NUM_PORTS; i++)
    {
        close(server_fd[i]);
    }

    return 0;
}
