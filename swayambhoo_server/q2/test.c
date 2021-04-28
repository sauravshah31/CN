#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#define TST_MSG "test message"

int main(int argc, char *argv[]){
    int usfd, sfd;
    const uint16_t port_number = atoi(argv[1]);

    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd<0){
        perror("socket() unix");
        return -1;
    }
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket()");
        return -1;
    }

    
    int on = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0){
        perror("setsockopt()");
        return -1;
    }
    if(setsockopt(usfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0){
        perror("setsockopt() unix");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    addr.sin_port = htons(port_number);

    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("bind()");
        return -1;
    }

    

    addr.sin_family = AF_UNIX;
    if(bind(usfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("bind() unix");
        return -1;
    }



    if(listen(usfd, 5) < 0){
        perror("listen() unix");
        return -1;
    }
    if(listen(sfd, 5) < 0){
        perror("listen()");
        return -1;
    }

    pid_t pid = fork();
    
    if(pid == 0){
        close(sfd);
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(usfd, (struct sockaddr*) &caddr,&caddrlen);
        printf("unix connected\n");
        while(1){
            send(usfd, TST_MSG, strlen(TST_MSG), 0);
            sleep(5);
        }
    }else if(pid > 0){
        close(usfd);
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(sfd, (struct sockaddr*) &caddr,&caddrlen);
        printf("tcp connected\n");
        while(1){
            send(sfd, TST_MSG, strlen(TST_MSG), 0);
            sleep(5);
        }
    }

    close(usfd);
    close(sfd);
}