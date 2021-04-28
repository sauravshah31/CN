#include <stdio.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define SEMID        3
#define SEMLOCK     -1
#define SEMUNLOCK    1


int curr_seamaphores(){
    pid_t pid;
    int status;

    pid = fork();

    if(pid == -1){
        fprintf(stderr,"fork error : %s\n",strerror(errno));
        return -1;
    }else if(pid==0){
        printf("CURRENT SEAMAPHORES\n");
        execlp("ipcs","ipcs","-s",NULL);
        return -1;
    }
    
    pid = waitpid(pid, &status, 0);
    if(pid<0){
        fprintf(stderr,"wait error : %s\n",strerror(errno));
    }

    return 0;
}

int init_semaphore(key_t SEM_KEY){
    //get semaphore
    struct sembuf sb;
    int semid;


    semid = semget(SEM_KEY, 1, IPC_CREAT|IPC_EXCL|0666);

    if(semid>0){
        sb.sem_num = 0;
        sb.sem_op = SEMUNLOCK;
        sb.sem_flg = 0;
        //lock
        if(semop(semid, &sb, 1) == -1){

        }
    }else if(errno==EEXIST){
            printf("couldn't lock, retrying\n");
            sleep(1);
            semid = init_semaphore(SEM_KEY);
    }

    return semid;
}

int main(){
    /*
        semget
        semctl
        semop
    */

    key_t SEM_KEY;
    int semid;
    struct sembuf sb = {
		.sem_num = 0,
		.sem_op  = SEMLOCK,
		.sem_flg = SEM_UNDO,
	};

    curr_seamaphores();
    
    SEM_KEY = ftok("./semaphores.c", SEMID);

    if(SEM_KEY == -1){
        fprintf(stderr,"ftok erro : %s\n",strerror(errno));
        exit(-1);
    }
    curr_seamaphores();

    //get semaphore
    semid = init_semaphore(SEM_KEY);
    printf("semid obtained\n");
    if (semop(semid, &sb, 1) == -1) {
		fprintf(stderr,"error while locking : %s\n",strerror(errno));
        exit(-1);
		/* NOTREACHED */
	}
    printf("lock acquired\n");
    curr_seamaphores();
    printf("press enter to unlock\n");
    (void)getchar();
    
    //release
    sb.sem_op = SEMUNLOCK;
    if (semop(semid, &sb, 1) == -1) {
		fprintf(stderr,"error while unlocking : %s\n",strerror(errno));
        exit(-1);
		/* NOTREACHED */
	}
    printf("unlocked\n");

    exit(0);
}