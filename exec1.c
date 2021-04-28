#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv){
    pid_t pid;
    int status;
    int i;

    (void) argv;
    printf("CURRENT PROCESS\n");
    printf("ARGC : %d\n",argc);
    pid = fork();

    if(pid<0){
        printf("fork ERROR : %s\n",strerror(errno));
    }else if(pid==0){
        char *args[argc+1];
        for(i=0;i<argc;i++){
            args[i]="a";
        }
        args[argc]=NULL;
        execvp("./a.out",args);
        //execlp("./a.out","sd","sd","sd",NULL);
        printf("ERROR : %s\n",strerror(errno));
    }

    if((pid = waitpid(pid, &status, 0))<0){
        printf("ERROR PID : %s\n",strerror(errno));
    }
}