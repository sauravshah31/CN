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
    char sequence[3];

    //compile print_arg
    pid = fork();
    if(pid == -1){
        exit(-1);
    }else if(pid == 0){
        execlp("gcc","gcc","print_arg.c","-o","temp_print",NULL);
        fprintf(stderr, "ERROR : %s\n",strerror(errno));
    }

    pid = waitpid(pid, &status, 0);

    

    //print processes
    pid = fork();
    if(pid == -1){
        exit(-1);
    }else if(pid == 0){
        execlp("./temp_print","temp_print","PROCESS 1 ",NULL);
        fprintf(stderr, "ERROR : %s\n",strerror(errno));
    }

    pid = waitpid(pid, &status, 0);
    if(pid<0){
        exit(-1);
    }

    pid = fork();
    if(pid == -1){
        exit(-1);
    }else if(pid == 0){
        execlp("./temp_print","temp_print","PROCESS 2 ",NULL);
    }

    pid = waitpid(pid, &status, 0);
    if(pid<0){
        exit(-1);
    }

    pid = fork();
    if(pid == -1){
        exit(-1);
    }else if(pid == 0){
        execlp("./temp_print","temp_print","PROCESS 3 ",NULL);
    }

    pid = waitpid(pid, &status, 0);
    if(pid<0){
        exit(-1);
    }

    return 0;
}