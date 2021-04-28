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
    //void (*signal(int sig, void (*func)(int)))(int);

    pid_t pid;
    char command[100];
    int nbytes,status;
    
    if(signal(SIGINT, sigint_handler)==SIG_ERR){
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
            if(signal(SIGINT, SIG_DFL)==SIG_ERR){
                fprintf(stderr,"child signal error : %s\n",strerror(errno));
            }
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