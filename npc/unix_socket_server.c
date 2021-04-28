#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/socket_practise.socket"

int main(){
    int status, nsfd;
    struct sockaddr_un name;
    int usfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    char buffer[BUFSIZ];

    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    status = bind(usfd, (const struct sockaddr *) &name,sizeof(struct sockaddr_un));

    status = listen(usfd, 5);

    while(1){
        nsfd = accept(usfd, NULL, NULL);

        while(1){
            read(nsfd, buffer, BUFSIZ);

            if(strncmp("CLOSE",buffer,BUFSIZ) == 0)
                break;
            
            printf("%s",buffer);
        }

        printf("close command issued\n");
        strcpy(buffer,"Connection closed");
        write(nsfd, buffer, BUFSIZ);

        close(nsfd);
    }


}