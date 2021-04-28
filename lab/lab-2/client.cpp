/*
    Client 
    Connect to groups
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>

#include <pthread.h>

#include <vector>

#define MSG_SIZE        1024
#define GLOBAL_MSQ_KEY  11
#define G1_MSQ_KEY      12
#define G2_MSQ_KEY      13

struct msg_ds{
    char msg[MSG_SIZE];
    int sender=0;
};
typedef struct message {
    long mtype;
    msg_ds mtext;
}message;

struct input{
    char type;
    int pid;
};

void* handle_receive(void *arg){
    struct input * inp = (struct input * ) arg;
    int msqid;
    int msqkey;
    if(inp->type == 'G'){
        msqkey = GLOBAL_MSQ_KEY;
    }else if(inp->type == '1'){
        msqkey = G1_MSQ_KEY;
    }else if(inp->type == '2'){
        msqkey = G2_MSQ_KEY;
    }else{
        printf("type error\n");
        return NULL;
    }

    if((msqid=msgget(msqkey, 0)) == -1){
        printf("msqid error\n");
        return NULL;
    }
    message msg;

    while(1){
        if(msgrcv(msqid,&msg,sizeof(msg),getpid(),0) != -1){
            printf("Message received from %d\n",msg.mtext.sender);
            printf("%s\n",msg.mtext.msg);
        }
    }
}

void* handle_send(void *arg){
    struct input * inp = (struct input * ) arg;
    int msqid_grp,msqid_global;
    int msqkey;
    if(inp[1].type == '1'){
        msqkey = G1_MSQ_KEY;
    }else if(inp[1].type == '2'){
        msqkey = G2_MSQ_KEY;
    }else{
        printf("type error\n");
        return NULL;
    }

    if((msqid_global = msgget(GLOBAL_MSQ_KEY, 0)) == -1){
        printf("msqid error\n");
        return NULL;
    }

    if((msqid_grp = msgget(inp[1].type=='1'?G1_MSQ_KEY:G2_MSQ_KEY, 0)) == -1){
        printf("msqid error\n");
        return NULL;
    }

    char c;
    message msg;
    msg.mtext.sender = getpid();
    while(1){
        printf("Enter message\n");
        printf("Global? : ");
        scanf("%c",&c);
        if(c=='y'){
            msqkey = msqid_global;
            msg.mtype = inp[0].pid;
        }else if(c=='n'){
            msqkey = msqid_grp;
            msg.mtype = inp[1].pid;
        }else{
            break;
        }

        printf("Message: ");
        scanf("%s",msg.mtext.msg);
        (void)getc(stdin);
        if(msgsnd(msqkey, &msg, sizeof(msg),IPC_NOWAIT) == -1){
            printf("couldn't send message\n");
        }
    }
    return NULL;
}

int main(int argc, char **argv){
    if(argc != 4){
        printf("Invalid Arguments\n");
        printf("Usage : ./a.out group_global_pid, group1_pid group_type\n");
        exit(EXIT_FAILURE);
    }

    int i,pid;
    

    //connect to the groups
    printf("Connecting to server...\n");

    pthread_t threads[3];
    struct input args[2];

    pid = atoi(argv[1]);
    args[0].type = 'G';
    args[0].pid = pid;

    if(pid == -1){
        printf("Invalid connection : %s\n",argv[i]);
        exit(EXIT_FAILURE);
    }
    if(kill(pid,SIGUSR1) == -1){
        printf("Couldn't connect to %d\n",pid);
        exit(EXIT_FAILURE);
    }

    pid = atoi(argv[2]);
    args[1].type = argv[3][0];
    args[1].pid = pid;

    if(pid == -1){
        printf("Invalid connection : %s\n",argv[i]);
        exit(EXIT_FAILURE);
    }
    if(kill(pid,SIGUSR1) == -1){
        printf("Couldn't connect to %d\n",pid);
        exit(EXIT_FAILURE);
    }

    pthread_create(&threads[0],NULL,handle_receive,(void*) &args[0]);
    pthread_create(&threads[1],NULL,handle_receive,(void*) &args[1]);
    pthread_create(&threads[2],NULL,handle_send,(void*)args);

    pthread_join(threads[2],NULL);
    printf("Closing...\n");

    printf("Disconnecting...\n");
    kill(args[0].pid,SIGUSR1);
    kill(args[1].pid,SIGUSR1);


    //kill the receiver threads
    pthread_kill(threads[0],SIGINT);
    pthread_kill(threads[1],SIGINT);
}