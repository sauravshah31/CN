#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(){
    int status;
    pid_t pid1,pid2;
    char sequence[3];

    //compile print_arg
    pid1 = fork();
    if(pid1 == -1){
        exit(-1);
    }else if(pid1 == 0){
        execlp("gcc","gcc","print_arg.c","-o","temp_print",NULL);
        fprintf(stderr, "ERROR : %s\n",strerror(errno));
    }



    pid1 = waitpid(pid1, &status, 0);

    

    //print processes
    pid1 = fork();
    if(pid1 == -1){
        exit(-1);
    }else if(pid1 == 0){
        execlp("./temp_print","temp_print","PROCESS 1 ",NULL);
        fprintf(stderr, "ERROR : %s\n",strerror(errno));
    }

    pid2 = fork();
    if(pid2 == -1){
        exit(-1);
    }else if(pid2 == 0){
        execlp("./temp_print","temp_print","PROCESS 2 ",NULL);
        fprintf(stderr, "ERROR : %s\n",strerror(errno));
    }

    pid1 = waitpid(pid1, &status, 0);
    pid2 = waitpid(pid2, &status, 0);

    return 0;
}