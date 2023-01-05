#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define IP "127.0.0.1"
#define PONG "pong!"
#define BUFSIZE 6

int main()
{
    struct timeval start, end;

    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0); // creating the listening socket to establish a connection
    if (listeningSocket <= 0)                              // Checking if the socket opened proparlly
    {
        perror("socket creation failed");
        close(listeningSocket);
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress)); // reset
    serverAddress.sin_family = AF_INET;               // IPv4
    serverAddress.sin_port = htons(3000);             // translates an integer from host byte order to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY;    // Convert Internet host address from numbers-and-dots notation in CP into binary data in network byte order.

    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) // 
    {
        perror("socket setting failed");
        close(listeningSocket);
        exit(1);
    }

    if (bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < -1) // we bind the socket with the settings we set above.
    {
        perror("binding failed");
        close(listeningSocket);
        exit(1);
    }

    if (listen(listeningSocket, 1) < 0) // here we "listen" and wait for a client to connect and by that start the communication.
    {
        perror("listening failed");
        close(listeningSocket);
        exit(1);
    }

    printf("Waiting for incoming TCP-connection...\n"); // we wait for the client to send a connection request.

    struct sockaddr_in clientAddress; // we set our preferences for the client socket
    socklen_t clientAddressLen = sizeof(clientAddress);
    memset(&clientAddress, 0, sizeof(clientAddress)); // reset
    clientAddressLen = sizeof(clientAddress);

    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen); // Await a connection on socket, When a connection arrives, open a new socket to communicate with it
    if (clientSocket <= 0)                                                                            // checks if the connection has done proparlly
    {
        perror("new_ping socket acception failed.");
        close(listeningSocket);
        exit(1);
    }

    printf("started watchdog with new_ping\n");

    char ip[16] = {0};

    if (recv(clientSocket, ip, 16, 0) < 0) // receiving PONG message
    {
        perror("recv() failed");
        close(clientSocket);
        exit(errno);
    }
    printf("1\n");

    while (1)
    {
        char okmsg1[BUFSIZE] = { 0 };
        if (recv(clientSocket, okmsg1, BUFSIZE, 0) < -1) // receiving PONG message
        {
            printf("recv() failed");
            close(clientSocket);
            exit(1);
        }
        printf("2\n");
        if (strcmp(okmsg1, PONG) != 0)
        {
            printf("Error");
            close(clientSocket);
            close(listeningSocket);
            exit(1);
        }
        char okmsg2[BUFSIZE] = { 0 };
        // Start the timer
        gettimeofday(&start, 0);
        if (recv(clientSocket, okmsg2, BUFSIZE, 0) < -1) // receiving PONG message
        {
            printf("recv() failed");
            close(clientSocket);
            close(listeningSocket);
            exit(1);
        }
        printf("3\n");
        if (strcmp(okmsg2, PONG) != 0)
        {
            printf("Error");
            close(clientSocket);
            close(listeningSocket);
            exit(1);
        }
        // End the timer
        gettimeofday(&end, 0);

        float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
        if (milliseconds >= 10.0)
        {
            printf("server %p cannot be reached.", ip);
            kill(0, 1);
        }

        settimeofday(&start, 0);
        settimeofday(&end, 0);
    }

    close(listeningSocket);
    close(clientSocket);

    return 0;
}