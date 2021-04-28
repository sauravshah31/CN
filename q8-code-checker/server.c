#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>

int main(){

    //make a fifo for request
    if(mkfifo("./REQUEST_FIFO",O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"connection fifo error : %s\n",strerror(errno));
            exit(-1);
        }
    }

    //make a fifo for output
    if(mkfifo("./RESPONSE_FIFO_STDOUT",O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"connection fifo error : %s\n",strerror(errno));
            exit(-1);
        }
    }

    //make a fifo for error
    if(mkfifo("./RESPONSE_FIFO_STDERR",O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"connection fifo error : %s\n",strerror(errno));
            exit(-1);
        }
    }

    printf("...Initializing Server\n");
    printf("Press ctrl+c to stop\n");
    while(1){
        int fd = open("./REQUEST_FIFO",O_RDONLY);
        int filefd = open("./temp.c",O_RDONLY| O_WRONLY | O_TRUNC);
        int nbytes=0;
        const size_t nchars = 1024;
        char msg[nchars];
        //write the code to a temp file
        while((nbytes=read(fd,msg,nchars)) >0)
                write(filefd,msg,nbytes);
        close(filefd);
        printf("REQUEST RECEIVED\n");
        FILE *fin = popen("gcc temp.c >RESPONSE_FIFO_STDERR;./a.out > RESPONSE_FIFO_STDOUT","r");
        
        close(fd);
        printf("RESPONSE PROCESSED\n");
    }
}