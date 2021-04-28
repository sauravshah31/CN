#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

#define SHM_KEY     1112
#define SHM_SIZE    1024


int main(int argc, char **argv){
    int shmid,fd;
    char *addr;

    if(argc <2){
        fprintf(stderr,"error : invalid arguemts\nusage : ./a.out mode[r/w] [data]");
    }
    //obtain a shared memory id
    if((shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT)) == -1){
        fprintf(stderr,"shmget error : %s",strerror(errno));
        return -1;
    }
    
    //attach shared memory to the address space
    addr = shmat(shmid, NULL, 0);

    if(addr == (void *) -1){
        fprintf(stderr,"shmat error : %s",strerror(errno));
        return -1;
    }



    if(argv[1][0]=='r'){
        fprintf(stdout,"%s",addr);
    }else if(argv[1][0]=='w' && argc==3){
        strncpy(addr, argv[2], SHM_SIZE);
    }

    (void)shmdt(addr);


    return 0;
}