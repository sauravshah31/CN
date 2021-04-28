#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv){

    int status;
    int sequence_no;
    pid_t pid;
    char sequence[3];

    if(argc==1){
        sequence_no = 0;
    }else{
        sequence_no = atoi(argv[1]);
    }
    
    if(sequence_no >= 4){
        printf("|\n");
        return 0;
    }

    sequence_no += 1;
    snprintf(sequence,sizeof(sequence),"%d",sequence_no);
    printf("Process %d -> ",sequence_no);
    fflush(stdout);
    
    pid = fork();
    if(pid<0){
        fprintf(stderr,"Error while forking : %s\n",
                strerror(errno));
    }else if(pid==0){
        execlp(argv[0],argv[0],sequence,NULL);
        fprintf(stderr,"Error while replacing : %s\n",
                strerror(errno));
    }

    //parent waits
    if((pid = waitpid(pid, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }

    return 0;
}