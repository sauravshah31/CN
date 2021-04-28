/*
    Send signals to threads

    usage
        gcc file_name.c -l pthread
        ./a.out
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>

pthread_t threads[5];

static void sig_handler(int signo, siginfo_t *info, void *ucontext){
    char msg[100];
    sprintf(msg,"\nreceived signal %d from %d"
            ,info->si_signo
            ,info->si_pid
    );

    //unbuffered io
    write(STDIN_FILENO,msg,strlen(msg));

}

void* background_thread(void *args){
    printf("background thread\n");

    pause();
}

int main(){
    int i;

    struct sigaction act = {
        .sa_sigaction = sig_handler,
        .sa_flags = SA_SIGINFO
    };

    printf("pid : %d\n",getpid());
    if(sigaction(SIGINT, &act, NULL)==-1){
        fprintf(stderr,"signal error : %s\n",strerror(errno));
        return -1;
    }

    for(i=0;i<5;i++){
        pthread_create(&threads[i],NULL,background_thread,NULL);
    }

    sleep(1);
    //send signals to all threads
    for(i=0;i<5;i++){
        pthread_kill(threads[i], SIGINT);
    }


    for(i=0;i<5;i++){
        pthread_join(threads[i],NULL);
    }

    exit(EXIT_SUCCESS);

}
