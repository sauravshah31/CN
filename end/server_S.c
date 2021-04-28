#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include <pthread.h>


//for closing the server
int isclosed = 0;
void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server...\n");
        isclosed = 1;
    }
}

//argument to pass to thread function
typedef struct args{
    int fd;
}args;

void* handle_client(void* arg){

    args *inp = (args*) arg;
    int nsfd = inp->fd;
    
    printf("Client connected...\n");
                        
    signal(SIGPIPE,SIG_IGN);
    char msg[1024];
    sprintf(msg, "Message from server");
    while(send(nsfd, msg, strlen(msg),0)>0){
        sleep(1);
    }
    close(nsfd);
    printf("Client disconnected\n");
    signal(SIGPIPE,SIG_DFL);
    close(nsfd);
}

int main(){
    const uint16_t port_number = 5000;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
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
    if(listen(sfd,10) < 0){
        printf("listen error : %s\n",strerror(errno));
        return -1;
    } 


    signal(SIGINT, sighandler);
    while(!isclosed){

        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);

        pthread_t thread;
        args arg = {
            .fd = nsfd
        };
        pthread_create(&thread ,NULL, handle_client,(void*) (&arg));

    }
    close(sfd);
}