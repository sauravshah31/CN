#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>



int main(int argc, char *argv[]){
    int sfd;
    const uint16_t port_number = atoi(argv[1]);

    sfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);

    if(connect(sfd, (const struct sockaddr *)&addr, sizeof(addr))< 0){
        perror("connect()");
        return -1;
    }
    printf("connected...\n");

    char buffer[1024];
    while(recv(sfd, buffer, 1024, 0) > 0){
        printf("%s\n",buffer);
    }
}