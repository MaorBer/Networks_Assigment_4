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


int main(int argc, char const *argv[])
{
    



    int timer = time(NULL);
    //printf(" %d",timer);
    while (1)
    {
         if (timer + 10 <= time(NULL))
        {
            printf("timeout %ld", (timer + 10 - time(NULL)));
            return 0;
        }
        
    }
    return 0;
}
