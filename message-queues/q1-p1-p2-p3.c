/*
A process P1 has to create a message queue 
and places several messages on the queue 
for processes P2 (type as 2 ) and P3 ( type as 3 ) 
and enquire about the state of the queue with msgctl(), 
and prints the many details of the message queue. 
Now Process P2 and P3 will read few of their messages. 
Now again process P1 has to inquire about the state of the queue 
with msgctl(), and prints the many details of the message queue. 
Next Process P1 has to do the below: 
It has to use msgctl() to alter (change ) a message on the queue.
Something like it may have to change the text of a message. 
And check whether the change has occurred by printing all the 
messages of the queue without msgrcv().
P has to use msgctl() to delete a message from the queue, 
then print all the messages of the queue to check whether a 
message has been deleted. Note that the message queue contents 
should not be changed after printing all the messages by using msgctl().
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define KEY 3
#define MSG_SIZE 50

typedef struct messsage {
    long mtype;
    char mtext[MSG_SIZE];
}messsage;

int main(){
    int status,msgid;

    msgid = msgget(KEY, IPC_CREAT | IPC_EXCL | 0600);

    if(msgid == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msgid = msgget(KEY, 0);
    }

    //P1 writes several messages to the queue
    int i=0;
    int choice;
    messsage msg; 
    for(i=0;i<10;i++){
        choice = rand() % 100;
        sprintf(msg.mtext,"Message %d\n",i);
        if(choice <= 60){
            //60% time, message for P2
            msg.mtype = 2;
        }else{
            //40% time, message for P3
            msg.mtype = 3;
        }
        status = msgsnd(msgid, &msg, strlen(msg.mtext)+1, IPC_NOWAIT);
        if(status == -1){
            fprintf(stderr,"msgsnd error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    //enquire about message queue
    struct msqid_ds qinfo;
    status = msgctl(msgid, IPC_STAT,&qinfo);
    if(status == -1){
        fprintf(stderr,"msgctl error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Message Queue Status\n");
    printf("Pid last sent : %d\n",qinfo.msg_lspid);
    printf("Last sent : %ld\n",qinfo.msg_stime);
    printf("Pid last received : %d\n",qinfo.msg_lrpid);
    printf("Last received : %ld\n",qinfo.msg_rtime);
    printf("Last changed : %ld\n",qinfo.msg_ctime);
    printf("Number of messages : %ld\n",qinfo.msg_qnum);
    printf("Max bytes allowed : %ld\n",qinfo.msg_qbytes);
    printf("\n");

    //P2 and P3 will read few of their messages
    
    printf("Process 2 reading\n");
    printf("------------------\n");
    for(i=0;i<3;i++){
        status = msgrcv(msgid, &msg, MSG_SIZE, 2, 0);
        if(status == -1){
            fprintf(stderr,"msgrcv error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("%s",msg.mtext);
    }
    printf("\n");

    printf("Process 3 reading\n");
    printf("------------------\n");
    for(i=0;i<1;i++){
        status = msgrcv(msgid, &msg, MSG_SIZE, 3, 0);
        if(status == -1){
            fprintf(stderr,"msgrcv error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("%s",msg.mtext);
    }
    printf("\n");

    //enquire again
    status = msgctl(msgid, IPC_STAT,&qinfo);
    if(status == -1){
        fprintf(stderr,"msgctl error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Message Queue Status\n");
    printf("Pid last sent : %d\n",qinfo.msg_lspid);
    printf("Last sent : %ld\n",qinfo.msg_stime);
    printf("Pid last received : %d\n",qinfo.msg_lrpid);
    printf("Last received : %ld\n",qinfo.msg_rtime);
    printf("Last changed : %ld\n",qinfo.msg_ctime);
    printf("Number of messages : %ld\n",qinfo.msg_qnum);
    printf("Max bytes allowed : %ld\n",qinfo.msg_qbytes);
    printf("\n");



    status = msgctl(msgid, IPC_RMID,NULL);

    if(status == -1){
        fprintf(stderr,"msgctl error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}