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

    while(1){
        int fd = open("./REQUEST_FIFO",O_WRONLY);
        char buffer[100];
        int nbytes;
        printf("Input\n");
        while((nbytes=read(STDIN_FILENO,buffer,100))>0){
            write(fd,buffer,nbytes);
            fflush(stdin);
        }
        
        close(fd);

        int err_fd = open("./RESPONSE_FIFO_STDERR",O_RDONLY);
        printf("$ gcc file.c\n");
        fflush(stdout);
        while((nbytes=read(err_fd,buffer,100))>0){
            write(STDOUT_FILENO,buffer,nbytes);
            fflush(stdout);
        }
        close(err_fd);

        printf("$ ./a.out\n");
        fflush(stdout);
        int output_fd = open("./RESPONSE_FIFO_STDOUT",O_RDONLY);
        while((nbytes=read(output_fd,buffer,100))>0){
            write(STDOUT_FILENO,buffer,nbytes);
            fflush(stdout);
        }

        close(output_fd);
        printf("\n");
    }
}