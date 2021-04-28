#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

void simple_fork(){
    pid_t process_id;
    int status;

    printf("CURRENT PROCESS \n");
    //create a new process
    process_id = fork();

    if(process_id == -1){
        printf("ERROR \n");
    }else if(process_id > 0){
        printf("PARENT : CHILD PID ->%d\n",process_id);
    }else{
        printf("CHILD \n");
        //terminate child process
        exit(0);
    }

    //wait for child to terminate
    if((process_id = waitpid(process_id, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }
}

void fork_memory(){
    
    pid_t process_id;
    int status;

    int variable = 3;
    printf("variable : %p : %d \n",&variable,variable);
    process_id = fork();

    if(process_id == -1){
        printf("ERROR \n");
    }else if(process_id > 0){
        printf("PARENT \n");
        printf("variable : %p : %d \n",&variable,variable);
        variable = 7;
        printf("variable : %p : %d \n",&variable,variable);
    }else{
        printf("CHILD \n");
        printf("variable : %p : %d \n",&variable,variable);
        variable = 9;
        printf("variable : %p : %d \n",&variable,variable);
        //terminate child process
        exit(0);
    }

    //wait for child to terminate
    if((process_id = waitpid(process_id, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }
}

int main(){
    printf("SIMPLE FORK\n");
    simple_fork();
    printf("\n");

    printf("FORK MEMORY\n");
    fork_memory();
    printf("\n");

    exit(0);
}