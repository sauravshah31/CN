#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define KEY 1
#define MSG_SIZE 1024

typedef struct messsage {
    struct list_head m_list;
    long mtype;
    char mtext[MSG_SIZE];
    struct msg_msgseg* next;
}messsage;


int main(){
    int status,msgid;

   
    msgid = msgget(KEY, 0);
    if(status == -1){
        fprintf(stderr,"msgget error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("msg id : %d\n",msgid);
    
    /*
        ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
                      int msgflg);
    */

    struct msqid_ds qinfo;
    struct msginfo *info;
    

    status = msgctl(msgid, IPC_INFO,&info);

    if(status == -1){
        printf("error");
        exit(-1);
    }

    info = info.
}