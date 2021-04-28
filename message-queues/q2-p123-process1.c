/*
    Process p1 communicated with other processes p2,p3,p4 using pid as mtype

    usage 
        ./a.out pid1 pid2 ...
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if(argc <= 1){
        fprintf(stderr,"invalid args\nusage: ./a.out pid1 pid2 ...");
        return -1;
    }

    int msgid;

    msgid = msgget(KEY, 0);

    if(msgid == -1){
        fprintf(stderr,"msgget error : %s\n",strerror(errno));
        return -1;
    }


    //send message to process
    int i,pid;
    messsage msg;
    for(i=1;i<argc;i++){
        pid = atoi(argv[i]);
        if(pid < 0){
            fprintf(stderr,"invalid argument %d\n",i);
            continue;
        }

        strcpy(msg.mtext,"You have a message");
        msg.mtype = pid;
        if(msgsnd(msgid, &msg, strlen(msg.mtext)+1, IPC_NOWAIT)){
            fprintf(stderr,"msgsnd err : %s\n",strerror(errno));
        }
    }
    

    struct msqid_ds qinfo;
    //wait till no message left 
    while(1){
        if(msgctl(msgid, IPC_STAT, &qinfo) == -1){
            fprintf(stderr,"msgctl error : %s\n",strerror(errno));
            break;
        }
        if(qinfo.msg_qnum == 0)
            break;
        sleep(1);
    };

    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}