/*
    helper (practise) before writing the actual code (lab-1.c)
    this is kinda rough practise

    two fifo : C1MESSAGE, C2MESSAGE

    parent listens to these fifo,
    if data is received, it write to a pipe

    child read from pipe and write into stdout

    Usage: 
        compile and run

        via another terminal, send message to C1MESSAGE : echo "message"| tee C1MESSAGE;
        simillarly to C2MESSAGE
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

void unbuffered_printf(int fd,int outfd){
    int nbytes = 0;
    char buffer[100];
    while((nbytes=read(fd,buffer,100)) > 0){
        write(outfd,buffer,nbytes);
    }
    if(nbytes == -1){
        write(STDOUT_FILENO,"write error\n",12);
    }
}

int main(){
    int status;
    printf("%d,%x\n",POLLHUP,POLLHUP);

    pid_t pid;
    int pipefd[2];

    if(pipe(pipefd) == -1){
        printf("pipe error\n");
        return -1;
    }

    pid = fork();

    if(pid > 0){
        close(pipefd[0]);

        struct pollfd fds[2] = 
        {
                {   
                    .fd = open("./C1MESSAGE",O_RDONLY | O_NONBLOCK),
                    .events = POLLIN,
                    .revents = 0
                },
                {
                    .fd = open("./C2MESSAGE",O_RDONLY | O_NONBLOCK),
                    .events = POLLIN,
                    .revents = 0
                }
        };
        

        while(1){
            write(STDOUT_FILENO,"listening...\n",13);
            status = poll(fds,2,-1);

            if(status > 0){
                printf("ready\n");
                fflush(stdout);
                if(fds[0].events & POLLIN){
                    unbuffered_printf(fds[0].fd,pipefd[1]);
                }
                if( fds[0].events & (POLLHUP | POLLERR | POLLIN)){
                    close(fds[0].fd);
                    fds[0].fd = open("./C1MESSAGE",O_RDONLY | O_NONBLOCK);
                }    
                
                if(fds[1].events & POLLIN){
                    unbuffered_printf(fds[1].fd,pipefd[1]);
                }
                if(fds[1].events & (POLLHUP | POLLERR | POLLIN)){
                    close(fds[1].fd);
                    fds[1].fd = open("./C2MESSAGE",O_RDONLY | O_NONBLOCK);
                }    
            }
        }
        wait(NULL);
    }else{
        close(pipefd[1]);
        unbuffered_printf(pipefd[0],STDOUT_FILENO);
        printf("Child\n");
    }
}