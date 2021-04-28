/*
    Server S
    Creates 3 groups (MSQueue)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <unordered_set>

#define MSG_SIZE        1024
#define GLOBAL_MSQ_KEY  11
#define G1_MSQ_KEY      12
#define G2_MSQ_KEY      13

std::unordered_set<int> CLIENTS;

struct msg_ds{
    char msg[MSG_SIZE];
    int sender=0;
};

typedef struct message {
    long mtype;
    msg_ds mtext;
}message;


static void init_client(int signo, siginfo_t *info, void *ucontext){
    int client_id = info->si_pid;
    printf("PID : %d :: ",getpid());
    if(CLIENTS.find(client_id) == CLIENTS.end()){
        printf("client %d connected\n",client_id);
        CLIENTS.insert(client_id);
    }else{
        printf("client %d disconnected\n",client_id);
        CLIENTS.erase(client_id);
    }
}

void broadcast_msg(int msqid,message *msg){
    int sender = msg->mtext.sender;
    for(int pid : CLIENTS){
        if(pid == sender){
            //don't send message to the sender
            continue;
        }
        msg->mtype = pid;
        if(msgsnd(msqid, &msg, sizeof(*msg), IPC_NOWAIT) == -1){
            fprintf(stderr,"Message not sent to client %d : %s\n",pid,strerror(errno));
        }
    }
}

void handle_group(int msqid){
    const pid_t gpid = getpid();
    printf("GROUP PID : %d\n",gpid);
    message msg;

    while(1){
        if(msgrcv(msqid, &msg, MSG_SIZE, gpid, 0) != -1){
            printf("Message received\n");
            broadcast_msg(msqid,&msg);
        }
    }
}

int main(){
    struct sigaction act;
    int status,msqid_g1, msqid_g2, msqid_global;
    pid_t pid1,pid2;


    //check is client is connecting
    act.sa_sigaction = init_client;
    act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGUSR1, &act, NULL) == -1){
        fprintf(stderr,"sigaction error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("SERVER PID : %d\n",getpid());

    //create Group1 msg queue
    msqid_g1 = msgget(G1_MSQ_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if(msqid_g1 == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msqid_g1 = msgget(G1_MSQ_KEY, 0);
    }

    //create Group2 msg queue
    msqid_g2 = msgget(G2_MSQ_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if(msqid_g2 == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msqid_g2 = msgget(G2_MSQ_KEY, 0);
    }

    //create global Group msg queue
    msqid_global = msgget(GLOBAL_MSQ_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if(msqid_global == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msqid_global = msgget(GLOBAL_MSQ_KEY, 0);
    }


    //process for handling g1
    pid1 = fork();

    if(pid1 > 0 ){
        //process for handling g2
        pid2 = fork();
        if(pid2>0){
            handle_group(msqid_global);
        }else if(pid2==0){
            handle_group(msqid_g2);
        }
    }else if(pid1 == 0){
        handle_group(msqid_g1);
    }

    (void)waitpid(pid1,&status,0);
    (void)waitpid(pid2,&status,0);

    printf("Closing server...\n");
    (void)msgctl(msqid_g1, IPC_RMID, NULL);
    (void)msgctl(msqid_g2, IPC_RMID, NULL);
    (void)msgctl(msqid_global, IPC_RMID, NULL);
}