#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

static void sigint_handler(int signo){
    printf("\nParent interrupt ignored\n");
    fflush(stdout);
}

static void sigusr2_handler(int signo){
    printf("\nSIGUSR2 interrupt,exiting\n");
    fflush(stdout);
    exit(-1);
}

int main(){
    pid_t pid;
    char command[100];
    int status,nbytes;

    printf("shell : %d\n",getpid());
    while(1){
        printf("%d $ ",getpid());
        fflush(stdout);
        nbytes = read(STDERR_FILENO,command,100);
        command[nbytes-1]='\0';
        pid = fork();
        if(pid==-1){
            fprintf(stderr,"fork error : %s\n",strerror(errno));
        }else if(pid==0){
            //custom handler for child process, exit on SIGUSR2
            signal(SIGUSR2, sigusr2_handler);
            printf("child : %d\n",getpid());
            fflush(stdout);
            execlp(command,command,NULL);
            fprintf(stderr,"exec error : %s\n",strerror(errno));
        }else{
            signal(SIGINT, sigint_handler);
            sleep(4);
            //if child take more than 4 seconds, send SIGUSR2 signal
            kill(pid,SIGUSR2);
            if(waitpid(pid,&status,0) == -1){
                fprintf(stderr,"wait error : %s\n",strerror(errno));
            }
            signal(SIGINT, SIG_DFL);
        }
    }
}