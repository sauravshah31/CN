/*
    child and parent duplex communication using pipes and threads
*/

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include <pthread.h>

struct input{
    int writefd;
    int readfd;
    char type;
};

void* read_t(void *arg){
    struct input *inp = (struct input *)arg;
    int nbytes;
    char buffer[1024];
    while((nbytes=read(inp->readfd,buffer,1024))>0){
        if(inp->type=='c'){
            printf("%10child reads\n",inp->type);
            printf("%10c%s",'\0',buffer);
        }else{
            printf("%carent reads\n",inp->type);
            printf("%s",buffer);
        }
    }

    return NULL;
}

void* write_t(void *arg){
    struct input *inp = (struct input *)arg;
    int nbytes;
    char buffer[1024];
    while((nbytes=read(STDIN_FILENO,buffer,1024))>0){
        if(inp->type=='c'){
            printf("%10child says\n",inp->type);
        }else{
            printf("%carent says\n",inp->type);
        }
        write(inp->writefd,buffer,nbytes);
    }

    return NULL;
}

int main(){
    int pipepc[2], pipecp[2];
    int status;
    int stdin_fd_copy, stdout_fd_copy;
    pid_t pid;

    status = pipe(pipepc);
    if(status==-1)printf("error");
    status = pipe(pipecp);
    if(status==-1)printf("error");

    
    pthread_t parent,child;

    struct input parent_input={
        .writefd = pipepc[1],
        .readfd = pipecp[0],
        .type = 'p' //parent
    };

    struct input child_input={
        .writefd = pipecp[1],
        .readfd = pipepc[0],
        .type = 'c' //child
    };

    //parent reads writes
    pthread_create(&parent, NULL, read_t, &parent_input);
    pthread_create(&parent, NULL, write_t, &parent_input);

    //child reads writes
    pthread_create(&child, NULL, read_t, &child_input);
    pthread_create(&child, NULL, write_t, &child_input);

    pthread_join(parent,NULL);
    pthread_join(child,NULL);

    return 0;

}