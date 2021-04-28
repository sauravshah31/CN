/*
    Send signal to a group of processes

    usage:
        ./a.out

*/

#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

static void sig_handler(int signo, siginfo_t *info, void *ucontext){
    char msg[100];
    sprintf(msg,"\n %d received signal %d from %d"
            ,getpid()
            ,info->si_signo
            ,info->si_pid
    );

   //unbuffered io
    write(STDIN_FILENO,msg,strlen(msg));

    if(info->si_signo == SIGUSR1){
        printf("\nexiting\n");
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

void create_process(int root_pid,int nprocess){
    if(nprocess <= 0){
        return;
    }
    pid_t pid = fork();
    if (pid>0){
        //parent
    }else if(pid==0){
        
        if (setpgid(getpid(), root_pid) == -1){
            fprintf(stderr,"setpgif error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("child process created : %d : gid = %d\n",getpid(),getpgrp());
        //create more process
        create_process(root_pid, nprocess-1);

        //wait for an interrupt
        pause();
        exit(-1);
    }
}

int main(){
    /*
        int killpg(int pgrp, int sig);
        int setpgid(pid_t pid, pid_t pgid);
    */

    int pid = getpid();
    struct sigaction act = {
        .sa_sigaction = sig_handler,
        .sa_flags = SA_SIGINFO
    };
    if(sigaction(SIGINT, &act, NULL)==-1){
        fprintf(stderr,"signal error : %s\n",strerror(errno));
        return -1;
    }

    printf("Root pid : %d , gid : %d\n",pid,getpgrp());
    fflush(stdout);

    //create 5 more process
    create_process(pid,5);

    if(getpid() == pid){
        //root process
        sleep(1);
        printf("press enter to send signal...\n");
        (void)getchar();
        //send interrupt SIGINT
        killpg(pid,SIGINT);
        wait(NULL);
    }

    printf("\n");
    sleep(1);
    while(1){
        printf("parent waiting...\nsend SIGUSR1 signal from another terminal to quit\n");
        //parent exits only when sigusr1 signal is received
        //this signal is to be send from another process (terminal)
        pause();
    }

}
