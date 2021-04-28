#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>

#define CLIENT_KEY 3
#define SERVER_KEY 4
#define MSG_SIZE 1024

typedef struct messsage {
    long mtype;
    char mtext[MSG_SIZE];
}messsage;

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr,"Invalid arguments\nUsage: ./a.out server_pid");
        return -1;
    }

    int server_pid = atoi(argv[1]);
    if(server_pid == -1){
        fprintf(stderr,"Invalid server pid\n");
        return -1;
    }

    //initialiaing connection
    if(kill(server_pid, SIGUSR1) == -1){
        fprintf(stderr,"Couldn't connect to server\n");
        return -1;
    }

    int pid = fork();
    if(pid == 0){
        //receive message
        int msgid = msgget(CLIENT_KEY, 0);
        if(msgid == -1){
            fprintf(stderr,"msgid error\n");
            exit(-1);
        }
        messsage msg;
        while(1){
            if(msgrcv(msgid, &msg, MSG_SIZE, getppid(),0) != -1){
                printf("Message received\n");
                printf("-----------------\n");
                printf("%s\n\n",msg.mtext);
                fflush(stdout);
            }
        }
    }else if(pid>0){
        //send message
        int msgid = msgget(SERVER_KEY, 0);
        if(msgid == -1){
            fprintf(stderr,"msgid error\n");
            exit(-1);
        }
        messsage msg;
        msg.mtype = getpid();
        while(read(STDIN_FILENO, msg.mtext,MSG_SIZE) > 0){
            if(msgsnd(msgid,&msg, strlen(msg.mtext)+1,IPC_NOWAIT) == -1){
                fprintf(stderr,"couldn't send message\n");
            }
        }

        //close connection
        if(kill(server_pid, SIGUSR1) == -1){
            fprintf(stderr,"Couldn't disconnect\n");
            exit(-1);
        }

        kill(pid,SIGINT);
        exit(0);
    }
}
