#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


static void sigint_handler(int signo){
    printf("\n$ ");
    fflush(stdout);
}

int main(){
    /*
        int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);
    */

    pid_t pid;
    char command[100];
    int nbytes,status;
    
    struct sigaction act = {
        .__sigaction_handler=sigint_handler
    };

    if(sigaction(SIGINT, &act, NULL)==-1){
        fprintf(stderr,"signal error : %s\n",strerror(errno));
        return -1;
    }

    printf("shell : %d\n",getpid());
    while(1){
        printf("$ ");
        fflush(stdout);
        nbytes = read(STDERR_FILENO,command,100);
        command[nbytes-1]='\0';
        pid = fork();
        if(pid==-1){
            fprintf(stderr,"fork error : %s\n",strerror(errno));
        }else if(pid==0){
            //default handler for child process
            execlp(command,command,NULL);
            fprintf(stderr,"exec error : %s\n",strerror(errno));
        }else{
            if(waitpid(pid,&status,0) == -1){
                fprintf(stderr,"wait error : %s\n",strerror(errno));
            }
        }
    }

    return 0;
}