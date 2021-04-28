#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]){
    if(argc !=3 ){
        fprintf(stderr,"Usage : %s dest_port_number bind_port_number",argv[0]);
        return -1;
    }

    const uint16_t port_number = atoi(argv[1]);
    const uint16_t bind_num = atoi(argv[2]);

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sfd<0){
        perror("socket()\n");
        return -1;
    }

    struct sockaddr_in baddr;
    baddr.sin_family = AF_INET;
    baddr.sin_addr.s_addr = htonl(INADDR_ANY);
    baddr.sin_port = htons(bind_num);

    if(bind(sfd, (struct sockaddr*) &baddr, sizeof(baddr))<0){
        perror("bind()\n");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);



    connect(sfd,(struct sockaddr*) &addr, sizeof(addr));

    struct sockaddr peeraddr;
    socklen_t peerlen;
    getpeername(sfd, &peeraddr, &peerlen);
    
    printf("Connected : %d , %s\n",peeraddr.sa_family,peeraddr.sa_data);

    char buf[1024];

    signal(SIGINT, sighandler);

    while(recv(sfd, buf, 1024, 0) > 0 && !isclosed){
        printf("%s\n",buf);    
    }
    close(sfd);

}