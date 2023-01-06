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

int main()
{
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
    serverAddress.sin_addr.s_addr = INADDR_ANY;       // Convert Internet host address from numbers-and-dots notation in CP into binary data in network byte order.

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
    if (recv(clientSocket, ip, 16, 0) < 0) // receiving the ip message
    {
        perror("recv() failed");
        close(clientSocket);
        exit(errno);
    }

    int timer = 0;
    char pong[5];
    // Time functionality
    while (timer < 10)
    {
        int bytes_received = recv(clientSocket, pong, 5, 0);    // Check if we received data
        if (bytes_received > 0)
        {
            timer = 0;
            continue;
        }
        else 
        {
            timer = timer + 1;
            sleep(1);
        }
    }

    printf("server %s cannot be reached.", ip);
    close(listeningSocket);
    close(clientSocket);
    kill(0,1);
    return 0;
}