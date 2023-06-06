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
#include <stdbool.h>
#include <netdb.h>

typedef struct
{
    float **M;
    int n;
    int start;
    int end;
    int port;
    char *ip_addr;
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
    // printf("\nPrint the %d x %d (+1) Matrix:\n", cols - 1, rows - 1);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // printf("%.3f ", matx[i][j]);
        }
        // printf("\n");
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

void get_host_name()
{
    char hostname[256];

    // Get the hostname
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("Error getting hostname");
        exit(1);
    }

    // Get host information by name
    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        perror("Error getting host information");
        exit(1);
    }

    // Convert the first IP address to a string and print
    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    char ip_address[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, addr_list[0], ip_address, INET_ADDRSTRLEN) == NULL)
    {
        perror("Error converting IP address");
        exit(1);
    }

    // printf("Current IP address: %s\n", ip_address);

    return;
}

int terrain_inter(float **matx, int row, int col)
{
    // printf("Calculating...\n");
    for (int i = 0; i < row; i++)
    {
        int min_x = getMin(i);
        int max_x = getMax(i);
        for (int j = 0; j < col; j++)
        {
            if (!((i % 10 == 0) && (j % 10 == 0)))
            {
                int min_y = getMin(j);
                int max_y = getMax(j);

                int area_d = (abs(min_x - i) * abs(min_y - j));
                int area_c = (abs(max_x - i) * abs(min_y - j));
                int area_b = (abs(min_x - i) * abs(max_y - j));
                int area_a = (abs(max_x - i) * abs(max_y - j));

                float elev_a = matx[min_x][min_y];
                float elev_b = matx[max_x][min_y];
                float elev_c = matx[min_x][max_y];
                float elev_d = matx[max_x][max_y];

                float elevation = ((area_a * elev_a) + (area_b * elev_b) + (area_c * elev_c) + (area_d * elev_d)) / (float)(area_a + area_b + area_c + area_d);

                matx[i][j] = elevation;
            }
        }
    }
    // printf("Matrix Calculation Done!\n");
    return 0;
}
// SLAVE: Receive MASTER messages
int handle_client(int client_socket, bool *calculation_finished)
{
    int recv_cols;
    int recv_rows;
    // Read col and row
    int bytesReceived1 = read(client_socket, &recv_cols, sizeof(int));
    if (bytesReceived1 == -1)
    {
        perror("Error reading recv_cols");
        // Handle the error, such as closing the socket and returning an error code
        close(client_socket);
        return 1;
    }
    else if (bytesReceived1 != sizeof(int))
    {
        // fprintf(stderr, "Incomplete read for recv_cols\n");
        // Handle the error, such as closing the socket and returning an error code
        close(client_socket);
        return 1;
    }

    int bytesReceived2 = read(client_socket, &recv_rows, sizeof(int));
    if (bytesReceived2 == -1)
    {
        perror("Error reading recv_rows");
        // Handle the error, such as closing the socket and returning an error code
        close(client_socket);
        return 1;
    }
    else if (bytesReceived2 != sizeof(int))
    {
        // fprintf(stderr, "Incomplete read for recv_rows\n");
        // Handle the error, such as closing the socket and returning an error code
        close(client_socket);
        return 1;
    }

    // Create matx template
    float **result = createMatx(recv_rows, recv_cols);

    printf("\nReceiving Matrix...\n");
    for (int row = 0; row < recv_rows; row++)
    {
        for (int col = 0; col < recv_cols; col++)
        {
            float recv_float;
            // Get received float element and store in matx
            int bytesReceived = read(client_socket, &recv_float, sizeof(float));
            if (bytesReceived == -1)
            {
                perror("Error reading float element");
                // Handle the error, such as closing the socket and returning an error code
                close(client_socket);
                return 1;
            }
            else if (bytesReceived != sizeof(float))
            {
                // fprintf(stderr, "Incomplete read for float element\n");
                // Handle the error, such as closing the socket and returning an error code
                close(client_socket);
                return 1;
            }
            result[row][col] = recv_float;
            // Clear buffer for next iteration
            memset(&recv_float, 0, sizeof(recv_float));
        }
    }

    printf("Matrix Received\n");

    // Interpolation
    terrain_inter(result, recv_rows, recv_cols);

    // Send acknowledgement
    char hello[10];
    // sprintf(hello, "ahckkkkk!");
    send(client_socket, hello, strlen(hello), 0);

    printf("\nSent ack back to client!\n");

    // Close the client socket
    close(client_socket);

    // Notify while loop that execution is done to close connection
    *calculation_finished = true;
    return 0;
}

// SLAVE: Start listening on PORT
void *start_server(int port)
{
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

    // printf("\nSlave listening on port %d\n", port);

    get_host_name();
    bool calculation_finished = false;
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
        // printf("Master connected on port %d\n", port);

        // Handle the client connection in a separate function or thread
        handle_client(client_socket, &calculation_finished);

        if (calculation_finished)
            break;
    }

    // printf("\nSlave listening on port %d has been closed.\n", port);

    // Close the server socket
    close(server_fd);

    return NULL;
}

// MASTER: Connect to SLAVE PORTS
void *conn_to_server(void *arg)
{
    MasterArgs *args = (MasterArgs *)arg;
    int port = args->port;
    char *ip_addr = args->ip_addr;
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    // Create a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // printf("\n Socket creation error \n");
        return NULL;
    }

    // char ip_address[16]; // Assuming a maximum IPv4 address length of 15 characters

    // // Obtain the IP address from user input or configuration file
    // printf("Enter the IP address: ");
    // scanf("%15s", ip_address);

    // int port;
    // printf("Enter PORT: ");
    // scanf("%d", &port);

    // Set up the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0)
    // if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        // printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }

    // Connect to the server
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        // printf("\nConnection Failed \n");
        return NULL;
    }

    // printf("Connected to slave in PORT %d\n", port);

    send(client_fd, &args->n, sizeof(int), 0);
    send(client_fd, &args->chunk, sizeof(int), 0);

    // printf("Sending Matrix...\n");
    for (int i = args->start; i < args->end; i++)
    {
        for (int j = 0; j < args->n; j++)
        {
            if (send(client_fd, &args->M[i][j], sizeof(float), 0) == -1)
            {
                perror("Error sending data");
                // Handle the error, e.g., return or exit the function
                return NULL;
            }
        }
    }
    // printf("Matrix sent!\n");

    char buffer[1024] = {0};
    valread = read(client_fd, buffer, sizeof(buffer));

    if (valread == -1)
    {
        perror("Error reading data");
        // Handle the error, e.g., return or exit the function
        return NULL;
    }
    else if (valread == 0)
    {
        // Handle the case when the connection is closed
        // printf("Connection closed by the Slave.\n");
        // Perform necessary cleanup or terminate the function
        return NULL;
    }

    // printf("Received Message from Slave: %s\n", buffer);

    // destroyMatx(matx, n);

    // Clear the buffer
    memset(buffer, 0, sizeof(buffer));
    // memset(array_recv, 0, sizeof(array_recv));

    // Close the connected socket
    close(client_fd);
    return NULL;
}

int main()
{
    int choice;
    // printf("Choose mode:\n");
    // printf("1. Slave\n");
    // printf("2. Master\n");
    // printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        int port;

        // printf("Enter your port: ");
        scanf("%d", &port);
        start_server(port);

        // printf("\nSlave mode ending.\n");
    }
    else if (choice == 2)
    {
        // int port;
        int num_slaves;
        int n = 0;
        srand(time(NULL));

        // printf("Enter PORT: ");
        // scanf("%d", &port);

        // printf("Enter the number of slaves to connect to: \n");
        scanf("%d", &num_slaves);

        // printf("Enter n: \n");
        int check = scanf("%d", &n);
        while (!check || n <= 9 || n % 10 != 0)
        {
            // printf("Wrong Input! Try again: \n");
            check = scanf("%d", &n);
        }

        // To compensate for extra 0-th coordinate
        n++;

        pthread_t threads[num_slaves];
        MasterArgs args[num_slaves];

        float **matx = createMatx(n, n);
        populateMatx(matx, n);

        int chunk_size = (n - 1) / num_slaves;

        struct timeval start_time, end_time;

        gettimeofday(&start_time, NULL);
        printf("Sending Matrix...\n");
        for (int i = 0; i < num_slaves; i++)
        {
            char ip_address[16]; // Assuming a maximum IPv4 address length of 15 characters
            int port;
            int min = getMin(i);
            int max = getMax(i);

            // Obtain the IP address from user input or configuration file
            // printf("Enter the IP address: \n");
            scanf("%15s", ip_address);

            // printf("Enter PORT: \n");
            scanf("%d", &port);

            args[i].M = matx;
            args[i].n = n;

            if (i == 0)
                args[i].start = 0;
            else
                args[i].start = getMin((i * chunk_size) + 1);

            args[i].end = getMax((i + 1) * chunk_size) + 1;
            args[i].port = port;
            args[i].ip_addr = ip_address;
            args[i].chunk = args[i].end - args[i].start;

            pthread_create(&threads[i], NULL, conn_to_server, &args[i]);
            // printf("Sent to IP and PORT: %s : %d\n", ip_address, port);

            // port++;
        }

        for (int i = 0; i < num_slaves; i++)
            pthread_join(threads[i], NULL);

        gettimeofday(&end_time, NULL);
        long seconds = end_time.tv_sec - start_time.tv_sec;
        long micros = ((seconds * 1000000) + end_time.tv_usec) - (start_time.tv_usec);
        double elapsed_time = (double)micros / 1000000.0;
        printf("\nCalculation Complete!\n\t> Matrix Size: %d x %d\n\t> Number of Slaves: %d\n\t> Elapsed Time (from sending to calculating): %.5f\n", n, n, num_slaves, elapsed_time);
        // printf("\nElapsed time: %.5f seconds\n", elapsed_time);

        printf("\nMaster mode ending.\n");
    }
    else
    {
        // printf("Invalid choice.\n");
        return 1;
    }

    return 0;
}