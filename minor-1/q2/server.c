#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define SHMID 12
#define MSGID 15

int msgno;

void write_for_client(){
    int shmid = shmget(SHMID, 1024, 0666|IPC_CREAT);
    if(shmid == -1)
        shmid = shmget(SHMID, 1024, 0);
    char *addr = shmat(shmid, NULL, 0);
    msgno = 0;
    sprintf(addr, "Message for client :%d\n", msgno++);
    shmat(shmid, addr, 0);
}

static void sighandler(int signo, siginfo_t *info, void *ucontext){
    if(info->si_signo = SIGUSR1){
        //some client connected
        //write data to shared memory
        printf("Client connected : %d\n",info->si_pid);
        write_for_client();
        //signal the child
        kill(info->si_pid, SIGUSR2);
    }
}

int main(){
    printf("Server id : %d\n",getpid());
    

    struct sigaction act = {
        .sa_sigaction = sighandler,
        .sa_flags = SA_SIGINFO
    };

    sigaction(SIGUSR1, &act, NULL);

    //initialize the message quque for clients
    msgget(MSGID,IPC_CREAT|0666);
    while(1){
        //wait till client wants to read
        pause();
    }
}