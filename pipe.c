/*
    parent writes dummy data into pipe, and childs reads it then printd into stdout
*/

#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


int main(){
    int pipefd[2];
    int status;
    pid_t pid;

    status = pipe(pipefd);
    if(status==-1){
        fprintf(stderr,"error while creating pipe : %s\n",strerror(errno));
        return -1;
    }

    pid = fork();

    if(pid==-1){
        fprintf(stderr,"fork error : %s\n",strerror(errno));
        (void)close(pipefd[0]);
        (void)close(pipefd[1]);
        return -1;
    }

    if(pid>0){
        //parent
        (void)close(pipefd[0]);
        sleep(2);
        printf("parents writes---\n");
        (void)write(pipefd[1],"dummy string from parent\n",34);
        (void)close(pipefd[1]);

        pid = waitpid(pid, &status, 0);
    }else{
        //child
        int nbytes;
        char buffer[1024];
        (void)close(pipefd[1]);
        while((nbytes=read(pipefd[0],buffer,1024))>0){
            printf("child reads---\n");
            (void)write(STDOUT_FILENO,buffer,nbytes);
        }
        (void)close(pipefd[0]);
    }

    return 0;
}