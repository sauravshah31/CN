#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>


#define SEMID        1
#define SEMLOCK     -1
#define SEMUNLOCK    1

void *receive_msg(void *inp){
    while(1){
        int fd = open("./MESSAGE_STORE",O_RDONLY);
        if(fd<0){
            sleep(1);
            continue;
        }
        int nbytes=0;
        const size_t nchars = 1024;
        char msg[nchars];
        while((nbytes=read(fd,msg,nchars))>0){
            printf("%10s",msg);
            fflush(stdout); 
        }
        printf("\n");
        close(fd);
    }
}

void *send_msg(void *inp){
    char *id = (char*) inp;
    while(1 && errno!=EINTR){
        int nbytes=0;
        size_t nchars = 1024;
        char *msg = malloc(sizeof(char)*nchars);
        int ofd = open(id,O_WRONLY);
        int fd = dup(STDIN_FILENO);
        getline(&msg,&nchars,stdin);
        /*
        while((nbytes=read(fd,msg,nchars))>0){
            //write(ofd,msg,nbytes);
        }*/
        //dprintf(ofd,"%d",getpid());
        dprintf(ofd,"%s",msg);
        close(ofd);
    }
    printf("OUT");
    unlink(id);
}


int main(){
    int sem_key,sem_id,fd;
    struct sembuf sb = {
		.sem_num = 0,
		.sem_op  = SEMLOCK,
		.sem_flg = SEM_UNDO,
	};
    sem_key = ftok("./server.c",SEMID);

    sem_id = semget(sem_key, 1, IPC_EXCL);

    
    
    //try to connect to the client (lock seamophore)
    printf("...Trying to connect\n");
    
    if (semop(sem_id, &sb, 1) == -1) {
		fprintf(stderr,"error while locking : %s\n",strerror(errno));
        exit(-1);
	}
    printf("Connection sucessfull\n\n");

    fd = open("./CONNECTION_FIFO",O_WRONLY);
    dprintf(fd,"%d",getpid());
    printf("%d\n",getpid());
    close(fd);
    printf("Type your message\n");
    
    //create handler threads
    pthread_t receiver,sender;

    char *id = malloc(sizeof(char)*10);
    sprintf(id,"%d",getpid());

    pthread_create(&receiver,NULL,&receive_msg,NULL);
    pthread_create(&sender,NULL,&send_msg,id);

    pthread_join(sender,NULL);
    pthread_cancel(receiver);

    
    return 0;
}