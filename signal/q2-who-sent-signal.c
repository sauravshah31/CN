/*
    print the process id who sent the signal

    Usage:
        ./a.out in one terminal
        from another terminal send signal to this abouve process (pid printed)
            kill -SIGNUM pid

        to terminate the process, send SIGUSR1
*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>


static void sig_handler(int signo, siginfo_t *info, void *ucontext){
    printf("\nsignal %d received from %d\n"
            ,info->si_signo
            ,info->si_pid
    );
    fflush(stdout);
    if(info->si_signo == SIGUSR1){
        printf("\nexiting\n");
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}


int main(){
    /*
        int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);

        struct sigaction {
            void     (*sa_handler)(int);
            void     (*sa_sigaction)(int, siginfo_t *, void *);
            sigset_t   sa_mask;
            int        sa_flags;
            void     (*sa_restorer)(void);
        };
    */

    struct sigaction act = {
        .sa_sigaction = sig_handler,
        .sa_flags = SA_SIGINFO
    };
    printf("pid :%d\n",getpid());

    //custom signal handler
    if(sigaction(SIGINT, &act, NULL) == -1){
        fprintf(stderr,"sigaction error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while(1){
        pause();
    }
}