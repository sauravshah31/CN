#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

void dummy_child(){
    printf("I am a dummy child\n");
}

void simple_exec(){
    pid_t pid;
    int status;

    printf("CURRENT PROCESS\n");
    pid = fork();

    if(pid<0){
        printf("fork ERROR : %s\n",strerror(errno));
    }else if(pid==0){
        execlp("./a.out","",NULL);
        printf("ERROR : %s\n",strerror(errno));
    }

    if((pid = waitpid(pid, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }
}

int main(){
    simple_exec();
}