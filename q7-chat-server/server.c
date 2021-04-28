#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>


#define SEMID        1
#define SEMLOCK     -1
#define SEMUNLOCK    1

#define MAX_CLIENT   20

typedef struct client{
    char *id;
} client;

pthread_mutex_t db_lock; //only one client can write at a time into the common chat db

void *handle_receive(void* inp){
    /*checks if the client has sent any message*/
    client *c = (client*) inp;

    //create a fifo named pid of the client
    if(mkfifo(c->id,O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"client fifo error : %s\n",strerror(errno));
            return NULL;
        }
    }

    //check if client has sent any message
    while(1){
        int fd = open(c->id,O_RDONLY);
        int nbytes=0;
        const size_t nchars = 1024;
        char msg[nchars];
        if(fd == -1){
            //client left
            printf("%s left\n",c->id);
            fflush(stdout);
            break;
        }
        nbytes=read(fd,msg,nchars);

        //message received, lock db for writing
        pthread_mutex_lock(&db_lock);
            int ofd = open("./MESSAGE_STORE",O_WRONLY);
            dprintf(ofd,"%s\n",c->id);
            write(ofd,msg,nbytes);
            while((nbytes=read(fd,msg,nchars)) >0)
                write(ofd,msg,nbytes);
            close(ofd);
        pthread_mutex_unlock(&db_lock);
        close(fd);
    }

    //delete the client fifo
    unlink(c->id);
}


int main(){
    int sem_key,sem_id;
    struct sembuf sb = {
		.sem_num = 0,
		.sem_op  = SEMUNLOCK,
		.sem_flg = SEM_UNDO,
	};
    pthread_t client_thread_receiver[MAX_CLIENT];

    int curr_threads = 0;

    pthread_mutex_init(&db_lock, NULL);

    //create a semaphore
    sem_key = ftok("./server.c",SEMID);
    if(sem_key == -1){
        fprintf(stderr,"ftok erro : %s\n",strerror(errno));
        exit(-1);
    }
    sem_id = semget(sem_key, 1, IPC_CREAT|IPC_EXCL|0666);
    
    

    //make a fifo for connection
    if(mkfifo("./CONNECTION_FIFO",O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"connection fifo error : %s\n",strerror(errno));
            exit(-1);
        }
    }

    //make a fifo for storing common db
    if(mkfifo("./MESSAGE_STORE",O_CREAT | O_EXCL | 0666) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"storage fifo error : %s\n",strerror(errno));
            exit(-1);
        }
    }

    printf("...Initializing Server\n");
    printf("Press ctrl+c to stop\n");
    fflush(stdout);
    //check if client is connecting
    while(1){
        //client can now connect, unlock
        (void) semop(sem_id, &sb, 1);

        int fd = open("./CONNECTION_FIFO",O_RDONLY);
        int nbytes;
        char data[10];
        nbytes = read(fd,data,sizeof(data));
        (void)close(fd);

        if(curr_threads >= MAX_CLIENT){
            fprintf(stderr,"MAX CLIENT LIMIT REACHED \n");
            continue;
        }

        //assign two threads, for receiving and sending chat message
        client *c = malloc(sizeof(client));
        c->id = malloc(nbytes+1);
        strcpy(c->id,data);
        c->id[nbytes]='\0';


        pthread_create(&(client_thread_receiver[curr_threads]), NULL, handle_receive, c);
       
        curr_threads++;
        printf("CLIENT %s CONNECTED\n",c->id);
    }

    //delete the client fifo
    unlink("./CONNECTION_FIFO");

    printf("...Server Closed\n");

    return 0;
}