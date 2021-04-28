#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <unordered_set>

#define CLIENT_KEY 3
#define SERVER_KEY 4
#define MSG_SIZE 1024


std::unordered_set<int> CLIENTS;

typedef struct messsage {
    long mtype;
    char mtext[MSG_SIZE];
}messsage;

static void init_client(int signo, siginfo_t *info, void *ucontext){
    int client_id = info->si_pid;
    if(CLIENTS.find(client_id) == CLIENTS.end()){
        printf("client %d connected\n",client_id);
        CLIENTS.insert(client_id);
    }else{
        printf("client %d disconnected\n",client_id);
        CLIENTS.erase(client_id);
    }

}

void send_msg(int msqid,char *send_msg, int exclude_pid){
    int i=0;
    messsage msg;
    int l = strlen(send_msg);
    strcpy(msg.mtext,send_msg);
    for(int pid : CLIENTS){
        if(pid == exclude_pid)
            continue;
        msg.mtype = pid;
        if(msgsnd(msqid, &msg, l+1, IPC_NOWAIT) == -1){
            fprintf(stderr,"Message not sent to client %d : %s\n",pid,strerror(errno));
        }
    }
}



void listen(int msgid_client, int msgid_server){
    /* 
        When a message is sent to server, send that to all the clients
    */
    
    struct sigaction act;
    act.sa_sigaction = init_client;
    act.sa_flags = SA_SIGINFO;

    //check is client is connecting
    if(sigaction(SIGUSR1, &act, NULL) == -1){
        fprintf(stderr,"sigaction error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    messsage msg;
    while(1){
        if(msgrcv(msgid_server, &msg, MSG_SIZE, 0, 0) != -1){
            send_msg(msgid_client, msg.mtext, msg.mtype);
        }
    }
}

int main(int argc, char **argv){
    /*
        When a client connects, SIGUSR1 is sent to server
    */
    printf("Server Address : %d\n",getpid());
    
    int status,msgid_client, msgid_server;

    msgid_client = msgget(CLIENT_KEY, IPC_CREAT | IPC_EXCL | 0600);

    if(msgid_client == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msgid_client = msgget(CLIENT_KEY, 0);
    }

    msgid_server = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL | 0600);

    if(msgid_server == -1){
        if(errno != EEXIST){
            fprintf(stderr,"msgget error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        //get the id, if already exists
        msgid_server = msgget(SERVER_KEY, 0);
    }

    
    listen(msgid_client, msgid_server);
    
    exit(EXIT_SUCCESS);
}