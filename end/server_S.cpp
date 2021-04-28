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

#include <unordered_set>


#define MAINTAINANCE_TIME       10

//keep track of clients connected
std::unordered_set<pthread_t> clients;

//for closing the server
int isclosed = 0;
void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server...\n");
        isclosed = 1;
    }
}

//fake maintainance emulator
void maintainance(){
    sleep(MAINTAINANCE_TIME);
}

//signal for maintainace : SIGUSR1
void maintainance_signal(int signum){
    //send signal to all client serving threads
    if(signum == SIGUSR1){
        for(auto t:clients)
            pthread_kill(t, SIGUSR2);
    }

    //do the maintainance
    maintainance();
}

//argument to pass to thread function
typedef struct args{
    pthread_t t;
    int fd;
}args;

void* handle_client(void* arg){

    args *inp = (args*) arg;
    int nsfd = inp->fd;
    pthread_t t = inp->t;
    
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

    clients.erase(t);

    return NULL;
}

int main(){

    //open a socket for client at port 5000
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


    //ctrl + c closes the server
    signal(SIGINT, sighandler);

    //ignore SIGUSR2
    signal(SIGUSR2, SIG_IGN);

    //handle maintainance signal
    signal(SIGUSR1, maintainance_signal);

    //check if client wants to connect
    while(!isclosed){
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);


        //accept client and serve
        pthread_t thread;
        args arg = {
            .t = thread,
            .fd = nsfd
        };
        pthread_create(&thread ,NULL, handle_client,(void*) (&arg));

    }
    close(sfd);
}