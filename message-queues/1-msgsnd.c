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
    long mtype;
    char mtext[MSG_SIZE];
}messsage;

int main(int argc, char **argv){

    if(argc !=2 ){
        fprintf(stderr,"Invalid arguments\nusage : ./a.out \"Message to be sent\"");
        exit(EXIT_FAILURE);
    }

    int status,msgid;

    /*
        IPC_CREAT | IPC_EXCL -> create message queue if not exists, else set errno to EEXISTS
        0600 -> can read & write ,other user no permission

    */
    msgid = msgget(KEY, IPC_CREAT | IPC_EXCL | 0600);

    if(msgid == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msgid = msgget(KEY, 0);
    }

    printf("msg id : %d\n",msgid);
    
    //int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

    messsage msg = {
        .mtype=1
    };
    (void)strcpy(msg.mtext, argv[1]);

    status = msgsnd(msgid, &msg, strlen(msg.mtext)+1,0);

    if(status < 0){
        fprintf(stderr,"msgsnd error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("%s\n",msg.mtext);
    exit(EXIT_SUCCESS);
}