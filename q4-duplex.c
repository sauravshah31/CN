/*
    Duplex communication using pipes
*/

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


int main(){
    int pipepc[2], pipecp[2];
    int status;
    int stdin_fd_copy, stdout_fd_copy;
    pid_t pid;

    status = pipe(pipepc);
    if(status==-1)printf("error");
    status = pipe(pipecp);
    if(status==-1)printf("error");

    pid = fork();
    if(pid==-1){
        fprintf(stderr,"fork error : %s\n",strerror(errno));
    }else if(pid>0){
        //parent
        (void)close(pipepc[0]);
        (void)close(pipecp[1]);
        (void)write(pipepc[1],"message from parent",21);
        (void)close(pipepc[1]);
        int nbytes;
        char buffer[50];
        while((nbytes=read(pipecp[0],buffer,50)) > 0){
            write(STDOUT_FILENO,buffer,nbytes);
        }

        
        (void)close(pipecp[0]);
        pid = waitpid(pid,&status,0);
    }else{
        //child
        (void)close(pipecp[0]);
        (void)close(pipepc[1]);
        (void)write(pipecp[1],"message from child",20);
        (void)close(pipecp[1]);
        int nbytes;
        char buffer[50];
        while((nbytes=read(pipepc[0],buffer,50)) > 0){
            write(STDOUT_FILENO,buffer,nbytes);
        }

        
        (void)close(pipepc[0]);
    }

    return 0;

}