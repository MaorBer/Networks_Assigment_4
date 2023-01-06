#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 3000
#define IP "127.0.0.1"

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

// Checksum algo
unsigned short calculate_checksum(unsigned short *paddress, int len);

// run 2 programs using fork + exec
// command: make clean && make all && ./partb
int main()
{
    int wdsock = socket(AF_INET, SOCK_STREAM, 0);
    if (wdsock == -1)
    {
        perror("socket creation failed\n");
        close(wdsock);
        exit(1);
    }
    printf("Created socket successfully\n");
    struct sockaddr_in watchdogAddress;
    memset(&watchdogAddress, 0, sizeof(watchdogAddress)); // reset
    watchdogAddress.sin_family = AF_INET;                 // IPv4
    watchdogAddress.sin_port = htons(PORT);               // translates an integer from host byte order to network byte order
    watchdogAddress.sin_addr.s_addr = INVALID_ANY;        // recives any IP as an address which can communicate with it.
    int con = connect(wdsock, (struct sockaddr *)&watchdogAddress, sizeof(watchdogAddress));
    if (con == -1)
    {
        perror("connection creation failed");
        close(wdsock);
        exit(1);
    }

    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;
    int status;
    int pid = fork();
    if (pid == 0)
    {
        printf("in child \n");
        execvp(args[0], args); // running watchdog
    }
    wait(&status); // waiting for child to finish before exiting
    printf("child exit status is: %d", status);

    struct sockaddr_in dest_in;
    struct timeval start, end;
    struct icmp icmphdr; // ICMP-header
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    dest_in.sin_family = AF_INET; // IPv4
    // The port is irrelevant for Networking and therefore was zeroed.
    // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    // Create raw socket for IP-RAW
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }
    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18;

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = 0;

    while (1)
    {
        char data[IP_MAXPACKET] = "Hello world we are Yuval and Maor.\n";
        int datalen = strlen(data) + 1;
        // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
        icmphdr.icmp_cksum = 0;
        // Combine the packet
        char packet[IP_MAXPACKET];
        // Next, ICMP header
        memcpy((packet), &icmphdr, ICMP_HDRLEN);
        // After ICMP header, add the ICMP data.
        memcpy(packet + ICMP_HDRLEN, data, datalen);
        // Calculate the ICMP header checksum
        icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
        memcpy((packet), &icmphdr, ICMP_HDRLEN);

        gettimeofday(&start, 0); // Start the timer
        int bytes_sent = -1;
        // Send the packet using sendto() for sending datagrams.
        bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
        if (bytes_sent == -1)
        {
            fprintf(stderr, "sendto() failed with error: %d\n", errno);
            return -1;
        }
        // Get the ping response
        bzero(packet, sizeof(packet));
        socklen_t len = sizeof(dest_in);
        ssize_t bytes_received = -1;
        while ((bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
        {
            if (bytes_received > 0)
            {
                printf("%ld bytes from %s: icmp_seq=%d ", bytes_received, inet_ntoa(dest_in.sin_addr), icmphdr.icmp_seq);
                break;
            }
            else
            {
                printf("Error in recvfrom()\n");
            }
        }
        icmphdr.icmp_seq++;
        gettimeofday(&end, 0); // End the timer
        float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
        unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
        printf("RTT: %f milliseconds (%ld microseconds)\n", milliseconds, microseconds);
        settimeofday(&start, 0);
        settimeofday(&end, 0);

        // Make the ping program sleep some time before sending another ICMP ECHO packet.
        usleep(1000000);
    }

    // Close the raw socket descriptor.
    close(sock);
    close(wdsock);

    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}