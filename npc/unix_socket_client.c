#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/socket_practise.socket"

int main(){
    int usfd,status;
    struct sockaddr_un addr;
    char buffer[BUFSIZ];

    usfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    memset(&addr, 0, sizeof(struct sockaddr_un));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    status = connect (usfd, (const struct sockaddr *) &addr,sizeof(struct sockaddr_un));

    while(read(STDIN_FILENO, buffer, BUFSIZ)>0){
        write(usfd, buffer, BUFSIZ);
    }

    strcpy(buffer, "CLOSE");
    write(usfd, buffer, BUFSIZ);

    read(usfd, buffer, BUFSIZ);
    printf("%s\n",buffer);

    close(usfd);
    return 0;
}