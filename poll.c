#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    struct pollfd fds[1]={
        {
            .fd=open("./C1MESSAGE",O_RDONLY | O_NONBLOCK),
            .events=POLLIN
        }
    };
    int status;
    while(1){
        status = poll(fds,1,3000);
        printf("Initializing ...\n");
        fflush(stdout);

        if(status==-1){
            printf("error");
        }

        if(status > 0){
            printf("ready\n");
        }else if(status == 0){
            printf("timeout\n");
        }
    }

}