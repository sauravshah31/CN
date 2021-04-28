/*
    Process p1 communicated with other processes p2,p3,p4 using pid as mtype

    usage 
        ./a.out 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <errno.h>


#define KEY 2
#define MSG_SIZE 1024

typedef struct messsage {
    long mtype;
    char mtext[MSG_SIZE];
}messsage;

int main(int argc, char **argv){
    int msgid;
    int pid=getpid();

    printf("Pid : %d\n",pid);
    msgid = msgget(KEY, IPC_CREAT | IPC_EXCL | 0600);

    if(msgid == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            return -1;
        }
        msgid = msgget(KEY, 0);
    }


    messsage msg;
    if(msgrcv(msgid, &msg, MSG_SIZE, pid, 0) == -1){
        fprintf(stderr,"msgrcv error : %s\n",strerror(errno));
        return -1;
    }
    printf("Message received\n");
    printf("-----------------\n");
    printf("%s",msg.mtext);

    

    return 0;
}