#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define SHMID 12
#define MSGID 15

typedef struct message {
    long mtype;
    char mtext[1024];
}message;


void read_(){
    char *addr;
    int shmid = shmget(SHMID, 1024, 0);
    addr = shmat(shmid, NULL, 0);
    printf("%s\n",addr);
    shmdt(addr);
}

void handler(int signum){
    if(signum == SIGUSR2){
        //Now, I can read from server
        read_();
    }
}

int main(int argc, char *argv[]){
    if(argc!=2){
        printf("server pid not provided\n");
        return -1;
    }
    int server_pid = atoi(argv[1]);
    printf("client id : %d\n",getpid());

    //register to the queue
    int msgid = msgget(MSGID, 0);
    message msg;
    msg.mtype = getpid();
    msgsnd(msgid, &msg, 1024, 0);

    
    signal(SIGUSR2, handler);

    //wait for signal from previous client
    pause();

    printf("waiting for other client to connect\n");
    //read done, now wait till you can handle control to another client
    msgrcv(msgid, &msg, 1024, 0, 0);

    //signal the client that you can read
    kill(msg.mtype, SIGUSR2);
}