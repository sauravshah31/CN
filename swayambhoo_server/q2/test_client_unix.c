#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>


int main(int argc, char *argv[]){
    int usfd;
    const uint16_t port_number = atoi(argv[1]);

    usfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_UNIX;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);

    if(connect(usfd, (const struct sockaddr *)&addr, sizeof(addr))< 0){
        perror("connect()");
        return -1;
    }

    char buffer[1024];
    while(recv(usfd, buffer, 1024, 0) > 0){
        printf("%s\n",buffer);
    }
}