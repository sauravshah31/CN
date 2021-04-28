/*
    client-server communication using readv, writev

    client -> sends data to be processed to the server
    server -> processes data in chunks parallely in multiple thread 
    -----------
    usage
        ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <cmath>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static void* process_data(void *arg){
    char **addr = (char **)arg;
    //int num = atoi(*addr);
    int num = 4;
    printf("%p\n",addr);
    //do some processing
    if(num==-1){
        return NULL;
    }
    num += 4;
    //sprintf((char*) arg,"%d",num);
    return NULL;
}

int main(){
    pid_t pid;
    int pipeInput[2];
    int pipeOutput[2];
    pipe(pipeInput);
    pipe(pipeOutput);

    pid = fork();
    if(pid > 0){
        //server
        close(pipeInput[1]);
        close(pipeOutput[0]);
        const int max_chunks = 32;
        char input[max_chunks][33]; //32 threads each process 32 bits,at max
        struct iovec iov[max_chunks];
        int i;
        int nbytes;

        for(i = 0;i<max_chunks; i++){
            iov[i].iov_base = input[i];
            iov[i].iov_len = 32;
        }
        while(1){
            //check if client has sent data
            nbytes = readv(pipeInput[0],iov,max_chunks);
            printf("Data received\nProcessing\n");
            const int size = ceil(nbytes / 32.0); //each thread processes 32 bytes
            pthread_t threads[size];
            for(i=0;i<size;i++){
                printf("%p\n",pipeInput );
                pthread_create(&threads[i],NULL,process_data,(void *)(&input[i]));
            }

            for(i=0;i<size;i++)
                pthread_join(threads[i],NULL);
            
            //data processed, send to child
            int last_size = size - floor(nbytes / 32);
            iov[size-1].iov_len = last_size==0?32:last_size;
            writev(pipeOutput[1],iov,size);

        }
        wait(NULL);
    }else{
        //client
        close(pipeInput[0]);
        close(pipeOutput[1]);
        char data_to_be_processed[1025];
        int nbytes;
        while(1){
            printf("Enter data to be processed\n=> ");
            fgets(data_to_be_processed,1025,stdin);
            nbytes = strlen(data_to_be_processed);
            write(pipeInput[1],data_to_be_processed,nbytes);
            nbytes = read(pipeOutput[0],data_to_be_processed,1024);
            printf("data processed\n%s\n\n",data_to_be_processed);
        }
    }
    
}