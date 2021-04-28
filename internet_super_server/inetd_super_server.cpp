#include <stdio.h>
#include <vector>
#include <cstring>

#include <errno.h>

#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

struct config{
    vector<int> ports;
    vector<int> backlogs;
    vector<int> type;
    vector<char*> service;
};

void read_config(char *pathname, config &c){
    FILE *fin = fopen(pathname, "r");
    int n=0;
    long status;
    char type[4];
    while(true){
        c.ports.push_back(0);
        c.backlogs.push_back(0);
        c.service.push_back(new char[100]);
        status = fscanf(fin,"%d %s %d %s\n",&(c.ports[n]), type,&c.backlogs[n], c.service[n]);

        if(strncmp(type,"tcp",3) == 0){
            c.type.push_back(SOCK_STREAM);
        }else{
            c.type.push_back(SOCK_DGRAM);
        }
        

        if(status == EOF)
            break;
        n++;
    }
}

int handle_tcp(int sfd, char* service){
    int nsfd = accept(sfd, NULL, NULL);

    pid_t pid = fork();
    if(pid > 0){
        close(nsfd);
        return 0;
    }else if(pid == 0){
        close(sfd);
        dup2(nsfd, STDIN_FILENO);
        dup2(STDOUT_FILENO, nsfd);

        execl(service, service, NULL);
    }

    return -1;
}

int handle_udp(int sfd, char* service){
    pid_t pid = fork();
    if(pid > 0){
        return 0;
    }else if(pid == 0){
        int nsfd = dup(sfd);
        dup2(nsfd, STDIN_FILENO);
        dup2(STDOUT_FILENO, nsfd);
        execl(service, service, NULL);
    }

    return -1;
}

int handle_client(int sfd, char* service, int type){
    printf("client connected\n");
    if(type==SOCK_STREAM){
        return handle_tcp(sfd, service);
    }else{
        return handle_udp(sfd, service);
    }
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("argument error\n");
        printf("Usage: ./a.out config.txt\n");
        return -1;
    }

    config C;
    read_config(argv[1],C);

    vector<int> sfds(C.ports.size());

    for(int i=0;i<C.ports.size();i++){
        sfds[i] = socket(AF_INET, C.type[i], 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        addr.sin_port = htons(C.ports[i]);

        if(bind(sfds[i], (struct sockaddr*) &addr, sizeof(addr))<0){
            printf("bind err : %s\n",strerror(errno));
            return -1;
        }
        if(C.type[i]==SOCK_STREAM && listen(sfds[i],C.backlogs[i]) < 0){
            printf("listen error : %s\n",strerror(errno));
            return -1;
        }
    }

    fd_set rfds;
    FD_ZERO(&rfds);

    for(int i=0;i<sfds.size();i++){
        FD_SET(sfds[i], &rfds);
    }

    while(1){
        printf("...listening\n");
        fd_set watchfd = rfds;

        int ret = select(sfds[sfds.size()-1],&watchfd,NULL,NULL, NULL);


        if(ret > 0){
            printf("here\n");
            for(int i=0;i<sfds.size();i++){
                if(FD_ISSET(sfds[i], &watchfd)){
                    handle_client(sfds[i], C.service[i], C.type[i]);
                }
            }
        }
    }

    


}