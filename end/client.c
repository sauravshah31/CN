#include <stdio.h>
#include <string.h>

#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

int isclosed = 0;

void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing connection\n");
        isclosed = 1;
    }
}

int main(){
    const uint16_t port_number = 5000;

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);

    connect(sfd,(struct sockaddr*) &addr, sizeof(addr));

    printf("Connected...");

    char buf[1024];

    signal(SIGINT, sighandler);

    while(recv(sfd, buf, 1024, 0) > 0 && !isclosed){
        printf("%s\n",buf);    
    }
    close(sfd);

}