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



void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server\n");
        isclosed = 1;
    }
}

//argument to pass to thread function
typedef struct args{
    int fd;
}args;

void* handle_client(void* arg){

    args *inp = (args*) arg;
    int i = inp->service_no;
    int nsfd = inp->fd;
    
    printf("Client connected\n");
                        
    signal(SIGPIPE,SIG_IGN);
    char msg[1024];
    sprintf(msg, "Message from server : service %d",i);
    while(send(nsfd, msg, strlen(msg),0)>0){
        sleep(1);
    }
    close(nsfd);
    printf("Client disconnected\n");
    signal(SIGPIPE,SIG_DFL);
    close(nsfd);
}

int main(){
    const int backlogs[NSERVICE] = {3,3,3,3};
    const uint16_t port_numbers[NSERVICE] = {5000,5001,5002,5003};
    int sfds[NSERVICE];

    for(int i=0;i<NSERVICE;i++){
        int sfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sfd<0){
            printf("socket error : %sn",strerror(errno));
            return -1;
        }
        sfds[i] = sfd;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    for(int i=0;i<NSERVICE;i++){    
        addr.sin_port = htons(port_numbers[i]);

        if(bind(sfds[i], (struct sockaddr*) &addr, sizeof(addr))<0){
            printf("bind err : %s\n",strerror(errno));
            return -1;
        }
        if(listen(sfds[i],backlogs[i]) < 0){
            printf("listen error : %s\n",strerror(errno));
            return -1;
        } 
    } 

    struct pollfd pollfds[NSERVICE];
    for(int i=0;i<NSERVICE;i++){
        pollfds[i].fd = sfds[i];
        pollfds[i].events = POLLIN;
    }

    signal(SIGINT, sighandler);
    while(!isclosed){
        printf("listening...\n");

        int status = poll(pollfds,NSERVICE,-1);
        if(status == -1){
            printf("poll error\n");
            continue;
        }

        if(status > 0){
            for(int i=0;i<NSERVICE;i++){
                if(pollfds[i].revents & POLLIN){
                    int sfd = sfds[i];
                    struct sockaddr_in caddr;
                    socklen_t caddrlen = sizeof(caddr);
                    int nsfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);

                    pthread_t thread;
                    args arg = {
                        .fd = nsfd,
                        .service_no = i
                    };
                    pthread_create(&thread ,NULL, handle_client,(void*) (&arg));

                    
                }
            }
        }
        
    }
    
    for(int i=0;i<NSERVICE;i++)
        close(sfds[i]);
}