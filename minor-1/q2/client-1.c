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

int main(int argc, char *argv[]){
    if(argc!=2){
        printf("server pid not provided\n");
        return -1;
    }
    int server_pid = atoi(argv[1]);
    printf("client id : %d\n",getpid());


    signal(SIGUSR2, SIG_IGN);
    
    //inform server that I am ready for reading
    kill(server_pid, SIGUSR1);

    //wait for server to respond
    //server writes to shared memory
    pause();

    //attach shared memory
    char *addr;
    int shmid = shmget(SHMID, 1024, 0);
    addr = shmat(shmid, NULL, 0);
    printf("%s\n",addr);
    shmdt(addr);

    printf("waiting for other client to connect\n");
    //wait till other client joins the queue, ie, wants to get sertver process
    int msgid = msgget(MSGID, 0);
    message msg;
    msgrcv(msgid, &msg, 1024, 0, 0);

    //signal the client that you can read
    kill(msg.mtype, SIGUSR2);
    
}