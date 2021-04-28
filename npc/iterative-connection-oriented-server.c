#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>


int sfd, nsfd;
int isclosed = 0;

void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server\n");
        close(sfd);
        close(nsfd);
        exit(-1);
        isclosed = 1;
    }
}

int main(){

    const int backlog = 3;
    const uint16_t port_number = 5000;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd<0){
        printf("socket error : %sn",strerror(errno));
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);

    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        printf("bind err : %s\n",strerror(errno));
        return -1;
    }
    if(listen(sfd,backlog) < 0){
        printf("listen error : %s\n",strerror(errno));
        return -1;
    }  
    char *msg = "Message from server";

    signal(SIGINT, sighandler);
    while(!isclosed){
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        nsfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);

        printf("Client connected\n");

        signal(SIGPIPE,SIG_IGN);
        while(send(nsfd, msg, strlen(msg),0)>0){
            sleep(1);
        }
        close(nsfd);
        printf("Client disconnected\n");
        signal(SIGPIPE,SIG_DFL);
    }
    close(sfd);
}