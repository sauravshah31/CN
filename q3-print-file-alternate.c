/*
    Print the contents of file n bytes at a time alternatively by two process
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SEMID       1
#define SEMLOCK     -1
#define SEMUNLOCK   1

int init_semaphore(key_t key){
    int semid;

    /*
        IPC_CREAT -> create key if doesn't exist
        IPC_EXCL -> fail is key already exists
        0666 -> for permission -> 0(file), 6(rw), 666(owner,group,world)
    */
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);

    if(semid>0){
        //set id created
    }else if(errno == EEXIST){
        //set id already exists
    }else{

    }
    return semid;
}



int main(int argc, char **argv){
    key_t SEM_KEY;
    pid_t pid;
    int status;
    int semid;
    int fd;
    int nbytes;
    char buffer[15];
    struct sembuf sb = {
		.sem_num = 0,
		.sem_op  = SEMLOCK,
		.sem_flg = SEM_UNDO,
	};

    if(argc !=2 ){
        fprintf(stderr,"Invalid Arguments\nUsage: ./a.out input_file.txt");
        exit(EXIT_FAILURE);
    }

    //open file
    fd = open(argv[1],O_RDONLY);

    //obtain sem key
    SEM_KEY = ftok(argv[1],SEMID);

    if(SEM_KEY == -1){
        fprintf(stderr,"ftok error : %s",strerror(errno));
        exit(EXIT_FAILURE);
    }


    //obtain semaphore, parent initially locked
    semid = init_semaphore(SEM_KEY);

    //fork to create a child process
    pid = fork();

    if(pid==-1){
        fprintf(stderr,"fork error : %s",strerror(errno));
        exit(EXIT_FAILURE);
    }else if(pid>0){
        //parent
        nbytes = read(fd, buffer, 10);
        printf("Parent\n");
        printf("%s",buffer);
        sb.sem_op = SEMUNLOCK;
        semop(semid, &sb, 1);
        sleep(1);
        while(1){
            //wait, lock
            sb.sem_op = SEMLOCK;
            semop(semid, &sb, 1);
            nbytes = read(fd, buffer, 10);

            if(nbytes==0){
                printf("Parent breaking\n");
                break;
            }
            printf("Parent\n");
            printf("%s",buffer);
            sb.sem_op = SEMUNLOCK;
            semop(semid, &sb, 1);
            sleep(1);
            
            //read

            //check eof
        }

        //wait for child to terminate
        pid = waitpid(pid, &status, 0);
        if(pid<0){
            fprintf(stderr,"wait error : %s",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }else{
        //child
        while(1){
            //wait, lock
            sb.sem_op = SEMLOCK;
            semop(semid, &sb, 1);
            nbytes = read(fd, buffer, 10);

            if(nbytes==0){
                printf("Child breaking\n");
                break;
            }
            printf("Child\n");
            printf("%s",buffer);
            sb.sem_op = SEMUNLOCK;
            semop(semid, &sb, 1);
            sleep(1);
            
            //read

            //check eof
        }
    }
    

    (void) close(fd);
    return 0;
}