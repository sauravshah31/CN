/*
        X
       / \
     P1  P2
       \ /
        Y
    P1 writes to X
    then P2 reads from X
    then p2 writes to y
    then p1 reads from X
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_KEY_X       12
#define SHM_KEY_Y       13
#define SHM_SIZE        1024

void parent_reads(char *addr){
  printf("\nparent reads\n");
  printf("%s\n",addr);
}

void parent_writes(char *addr){
  sprintf(addr,"Parent");
  sleep(1);
}

void child_reads(char *addr){
  printf("\nchild reads\n");
  printf("%s\n",addr);
}

void child_writes(char *addr){
  sprintf(addr,"Child");
  sleep(1);
}

int main(){
    int shmid_x,shmid_y;
    pid_t pid;

    shmid_x=shmget(SHM_KEY_X, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if(shmid_x==-1){
      if(errno!=EEXIST){
        fprintf(stderr,"shared memory error : %s\n",strerror(errno)); 
      }
    }

    shmid_y=shmget(SHM_KEY_Y, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if(shmid_y==-1){
      if(errno!=EEXIST){
        fprintf(stderr,"shared memory error : %s\n",strerror(errno)); 
      }
    }


    char * addr_x = shmat(shmid_x, NULL,0);
    char * addr_y = shmat(shmid_y, NULL, 0);
    
    pid = fork();
    if(pid==-1){
      fprintf(stderr,"fork error : %s\n",strerror(errno));
      exit(-1);
    }

    if(pid>0){
      printf("parent : %d\n",getpid());
      //parent - p1
      sigset_t set;
      int sig;
      
      sigemptyset(&set);
      //read
      parent_reads(addr_y);
      sigaddset(&set, SIGUSR1);

      //write
      parent_writes(addr_x);

      //signal
      //kill(pid,SIGUSR2);

      //wait for signal from child
      //signal
      sigwait(&set, &sig);
      //signal

      //read
      parent_reads(addr_y);

      wait(NULL);
    }else{
      //child
      printf("child : %d\n",getpid());
      sigset_t set;
      int sig;

      sigemptyset(&set);
      sigaddset(&set,SIGUSR2);

      //wait for signal from parent
      sigwait(&set, &sig);

      //read
      child_reads(addr_x);

      //write
      child_writes(addr_y);

      //signal
      //kill(getppid(),SIGUSR1);

      exit(0);
    }

  exit(0);
}