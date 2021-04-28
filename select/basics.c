#include <stdio.h>

#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(){
    fd_set rfds,wfds,efds;
    int fdr = open("./fifo1",O_RDONLY | O_NONBLOCK);
    int fdw = open("./fifo2",O_WRONLY | O_NONBLOCK);
    int fde = open("./fifo3",O_RDONLY | O_NONBLOCK);

    struct timeval tv;
    int retval;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    //FD_SET(STDIN_FILENO,&rfds);
    FD_SET(fdr,&rfds);
    //FD_SET(STDOUT_FILENO,&wfds);
    FD_SET(fdw,&rfds);
    FD_SET(STDERR_FILENO,&efds);
    FD_SET(fde,&efds);

    retval = select(fde+1,&rfds,&wfds,&efds,NULL);
    while(1){
        if(retval == -1){
            perror("select() error\n");
        }else if(retval){
            printf("ready for reading\n");
            printf("rval : %d\n",retval);
        }else{
            printf("timeout\n");
        }
    }

    return 0;
}