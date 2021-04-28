#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <errno.h>

#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>


#define PORTCP  6001
#define PORTDP  6002

#define PORT21 5021
#define PORT20 5020


int main(){
    int control_sfd, data_sfd;

    if((control_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket cp");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORTCP);

    if (bind(control_sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind cp");
        exit(-1);
    }
  

    if((data_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket cp");
        exit(-1);
    }

    addr.sin_port = htons(PORTDP);

    if (bind(data_sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind cp");
        exit(-1);
    }

    if (listen(data_sfd, 1) < 0)
    {
        perror("listen");
        exit(-1);
    }   


    addr.sin_port = htons(PORT21);

    if (connect(control_sfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        perror("connect cp");
        exit(-1);
    } 

    const int ms = pow(2,16);
    char buffer[ms];
    sprintf(buffer,"{\"port\":\"%d\"}",PORTDP);

    if(send(control_sfd, buffer, strlen(buffer), 0) < 0){
        perror("send cp");
    }
    if(recv(control_sfd, buffer, sizeof(buffer), 0) < 0){
        perror("recv cp");
    }

    if(strcmp(buffer,"ok") != 0){
        fprintf(stderr,"server rejected : %s\n",buffer);
        exit(-1);
    }

    printf("...ok...\n");
    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    int nsfd = accept(data_sfd, (struct sockaddr*) &caddr,&caddrlen);

    printf("...fetching files...\n");
    while(recv(nsfd, buffer, sizeof(buffer), 0) > 0){
        printf("%s",buffer);
    }

    printf("\n...completed...\n");
    close(control_sfd);
    close(data_sfd);

    return 0;
}