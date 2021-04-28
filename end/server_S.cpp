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

#define ALTERNATE_SERVER_ADDR    "0.0.0.0"
#define ALTERNATE_SERVER_PORT    7000

//keep track of clients connected
struct thread_client{
    pthread_t t;
    int fd;
};
std::unordered_set<thread_client *> clients;

//alternate server fd
int afd=-1;

//for closing the server
int isclosed = 0;
void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server...\n");
        isclosed = 1;
    }
}

//maintainance msg to client
void send_maintainance_msg(int fd){
    send(fd, "MAINT",6, 0);
}

//client details to alternate server
void send_client_as(int client){
    struct sockaddr addr;
    socklen_t addrlen;
    getpeername(client, &addr, &addrlen);

    char buf[1000];
    struct sockaddr_in s = *((struct sockaddr_in *) &addr);
    
    //send client ip,port to alternate server : in json format
    sprintf(buf,"{\"ip\":\"%d\",\"port\":\"%d\"}",s.sin_port, s.sin_addr.s_addr);

    send(afd, buf, strlen(buf),0);
}

//fake maintainance emulator
void maintainance(){
    sleep(MAINTAINANCE_TIME);
}

//signal for maintainace : SIGUSR1
void maintainance_signal(int signum){
    if(signum == SIGUSR1){
        for(auto t:clients){
            //send signal to all client serving threads to pause
            pthread_kill(t->t, SIGSTOP);

            //connect to alternate server
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(ALTERNATE_SERVER_PORT);
            //addr.sin_addr = inet_addr(ALTERNATE_SERVER_ADDR);
            if(connect(afd,(struct sockaddr*) &addr, sizeof(addr)) < 0){
                perror("connect");
                return -1;
            }

            //send msg to client that maintaince is goining on
            send_maintainance_msg(t->fd);

            //send msg to AS ragarding this client
            send_client_as(t->fd);
        }
    }

    //do the maintainance
    maintainance();
}


//argument to pass to thread function
typedef struct args{
    int fd;
    thread_client *ptr;
}args;

//function to serve client
void* handle_client(void* arg){

    args *inp = (args*) arg;
    int nsfd = inp->fd;
    thread_client *t = inp->ptr;
    
    printf("Client connected...\n");
    
    //ignore SIGUSR1
    signal(SIGUSR1, SIG_IGN);
    
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


    //client disconnected
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


    //connect to alternate server
    afd = socket(AF_INET, SOCK_STREAM, 0);
    if(afd<0){
        printf("socket error : %sn",strerror(errno));
        return -1;
    }

    
    


    //ctrl + c closes the server
    signal(SIGINT, sighandler);


    //handle maintainance signal
    signal(SIGUSR1, maintainance_signal);

    //check if client wants to connect
    while(!isclosed){
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);


        //accept client and serve

        pthread_t thread;

        thread_client *tmp = new thread_client();
        tmp->t = thread;
        tmp->fd = nsfd;
        args arg = {
            .fd = nsfd,
            .ptr = tmp
        };
        pthread_create(&thread ,NULL, handle_client,(void*) (&arg));

        clients.insert(tmp);
    }
    close(sfd);
}