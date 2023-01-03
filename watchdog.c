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

#define PORT 9696
#define IP "127.0.0.1"

int main()
{
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0); // creating the listening socket to establish a connection
    if (listeningSocket <= 0)                              // Checking if the socket opened proparlly
    {
        perror("socket creation failed");
        close(listeningSocket);
        exit(1);
    }

    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) // 
    {
        perror("socket setting failed");
        close(listeningSocket);
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress)); // reset
    serverAddress.sin_family = AF_INET;                // IPv4
    serverAddress.sin_port = htons(PORT);              // translates an integer from host byte order to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY;        // recives any IP as an address which can communicate with it.

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
        perror("client socket acception failed.");
        close(listeningSocket);
        exit(1);
    }

    printf("A new client connection accepted\n");








    int msec = 0, timer = 10; // 10 miliseconds
    clock_t before = clock();
    printf("hello partb");

    do
    {
        send();

        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
        // iterations++;
    } while (msec < timer);

    // printf("Time taken %d seconds %d milliseconds (%d iterations)\n",
        //    msec / 1000, msec % 1000, iterations);

    return 0;
}