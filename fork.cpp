#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>


int main(){
    pid_t process_id;

    printf("CURRENT PROCESS \n");
    //create a new process
    process_id = fork();

    if(process_id == -1){
        printf("ERROR \n");
    }else if(process_id > 0){
        printf("PARENT \n");
    }else{
        printf("CHILD \n");
    }
}