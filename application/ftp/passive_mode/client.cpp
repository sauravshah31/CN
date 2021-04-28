#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unordered_map>
#include <string.h>

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
        perror("socket dp");
        exit(-1);
    }

    addr.sin_port = htons(PORTDP);

    if (bind(data_sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind dp");
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
    sprintf(buffer,"{\"mode\":\"PASSV\"}");

    if(send(control_sfd, buffer, strlen(buffer), 0) < 0){
        perror("send cp");
    }
    if(recv(control_sfd, buffer, sizeof(buffer), 0) < 0){
        perror("recv cp");
    }
    printf("...ok...\n");
    auto dict = to_dict(buffer);


    int port = 0;
    for(char c:dict["port"]){
        port = port * 10 + (c-'0');
    }
    int ip = 0;
    for(char c:dict["ip"]){
        ip = ip * 10 + (c-'0');
    }
    printf("Data Server IP : %d\n",ip);
    printf("Data Server Port : %d\n",port);

    
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(ip);

    
    if (connect(data_sfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        perror("connect dp");
        exit(-1);
    } 

    printf("...fetching files...\n");
    while(recv(data_sfd, buffer, sizeof(buffer), 0) > 0){
        printf("%s",buffer);
    }

    printf("\n...completed...\n");
    close(control_sfd);
    close(data_sfd);

    return 0;
}