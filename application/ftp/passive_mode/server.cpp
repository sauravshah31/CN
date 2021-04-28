#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>


#define PORT21 5021


using namespace std;

unordered_map<string,string> to_dict(char *b){
    int i=1;
    unordered_map<string,string> dict;
    string key,value;
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


int serve(int sfd){
    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    int nsfd = accept(sfd, (struct sockaddr*) &caddr,&caddrlen);
    printf("here\n");
    const int ms = pow(2,16);
    char buffer[ms];
    int fd = open("../file.txt",O_RDONLY);
    int s;
    while((s = read(fd, buffer, sizeof(buffer))) > 0){
        send(nsfd, buffer, s, 0);
    }

    close(fd);
    close(nsfd);
    close(sfd);
    return 0;
}


int main()
{
    int control_sfd;
    if((control_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket cp");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT21);

    if (bind(control_sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind cp");
        exit(-1);
    }

    if (listen(control_sfd, 5) < 0)
    {
        perror("listen cp");
        exit(-1);
    }
    const int ms = pow(2,16);
    char buffer[ms];

    while(1){
        struct sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        int nsfd = accept(control_sfd, (struct sockaddr*) &addr,&caddrlen);
        if(recv(nsfd, buffer, sizeof(buffer), 0) < 0){
            perror("recv cp");
            close(nsfd);
            continue;
        }

        auto dict = to_dict(buffer);
        auto port = dict.find("mode");
        if(port == dict.end() || port->second != "PASSV"){
            sprintf(buffer, "Invalid request : mode is not PASSV\n");
            send(nsfd, buffer, strlen(buffer), 0);
            close(nsfd);
            continue;
        }

        int data_sfd;

        if((data_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket dp");
            exit(-1);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(0);

        if (bind(data_sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind dp");
            exit(-1);
        }

        sockaddr in;
        if (getsockname(data_sfd, &in, &caddrlen) < 0){
            perror("getsockname");
        }
        

        pid_t pid = fork();

        if(pid == 0){
            close(control_sfd);
            close(nsfd);
            serve(data_sfd);
            exit(1);
        }
        
        addr = *((sockaddr_in *) &in);
        close(data_sfd);
        

        sprintf(buffer,"{\"ip\":\"%d\",\"port\":\"%d\"}",addr.sin_addr.s_addr, addr.sin_port);

        send(nsfd, buffer, strlen(buffer), 0);
        close(nsfd);


    }

    
}