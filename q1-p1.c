#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


int main(){
    int status;
    pid_t pid;

    printf("PROCESS 1 -> ");
    fflush(stdout);
    sleep(1);
    pid = fork();
    if(pid<0){
        fprintf(stderr,"Error while forking : %s\n ",
                strerror(errno));
    }else if(pid==0){
        execlp("./q1-p2","a.out","",NULL);
        fprintf(stderr,"Error while replacing : %s\n",
                strerror(errno));
    }

    //parent waits
    if((pid = waitpid(pid, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }

    return 0;
}