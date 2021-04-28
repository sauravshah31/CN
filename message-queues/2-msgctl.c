/*
    Using msgctl
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define KEY 1


int main(){
    int status,msgid;

    msgid = msgget(KEY,0);
    if(msgid == -1){
        printf("Error");
        exit(-1);
    }
    /*
        int msgctl(int msqid, int cmd, struct msqid_ds *buf)
    */

    struct msqid_ds qinfo;

    status = msgctl(msgid, IPC_STAT,&qinfo);

    if(status == -1){
        printf("error");
        exit(-1);
    }

    printf("IPC_STAT\n");
    printf("Pid last sent : %d\n",qinfo.msg_lspid);
    printf("Last sent : %ld\n",qinfo.msg_stime);
    printf("Pid last received : %d\n",qinfo.msg_lrpid);
    printf("Last received : %ld\n",qinfo.msg_rtime);
    printf("Last changed : %ld\n",qinfo.msg_ctime);
    printf("Number of messages : %ld\n",qinfo.msg_qnum);
    printf("Max allowed : %ld\n",qinfo.msg_qbytes);
}