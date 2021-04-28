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
#include <unordered_map>
#include <string>


#define MAIN_SERVER_ADDR        "0.0.0.0"
#define MAIN_SERVER_PORT        5000 

//function to convert json to map
std::unordered_map<std::string,std::string> to_dict(char *b){
    int i=1;
    std::unordered_map<std::string,std::string> dict;
    std::string key,value;
    while(b[i] != '\0'){
        i += 1;
        key = "";
        value = "";
        while(b[i] != '"'){
            key += b[i++];
        }
        i += 3;
        while(b[i] != '"'){
            value += b[i++];
        }
        dict[key] = value;
        i += 2;
    }
    return dict;
} 


//for closing the server
int isclosed = 0;
void sighandler(int signum){
    if(signum == SIGINT){
        printf("\b\bClosing Server...\n");
        isclosed = 1;
    }
}

int main(){

    //create a socket for accepting response from main server
    const uint16_t port_number = 7000;
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

    

    //check if main server is sendng responses
    while (!isclosed){
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int msfd = accept(sfd, (struct sockaddr*) &addr,&caddrlen);

        close(msfd);
    }
    
    close(sfd);


}