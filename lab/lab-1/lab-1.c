/*
    'S' is a server process 
        has children
            A1,A2 in group G1
            B1,B2 in group G2
        
        S communicates with children in group G1 using pipe
        S communicates with children in group G2 using fd

    C1,C2 are other external processes,
        communicatting with S using named fifo
    
    D is another process, that can send signal to S

    When S receives message from C1/C2,
        initially it sends to group G1
    When D sends signal, the destination group is changed to G2
        meaning When S receives message from C1/C2,
        now it sends to group G2, then G1, ...

    Write the number of signals (kills) received into a shared memory
*/

/*
    Convention:
        when SIGUSR1 signal is received in 'S', assume D has sent signal
            => signal handler for SIGUSR1 
*/

/*
    usage:
        compile and run this script
            ./a.out
        send message to C1MESSAGE/C2MESSAGE via another terminal
            echo "message" |tee C1MESSAGE
            echo "message" |tee C2MESSAGE
        send SIGUSR1 interrupt to change the group via another terminal
            kill -SIGUSR1 pid

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define SHM_SIZE 32

int GROUPID1;
int GROUPID2;
int NKILLS=0;
char *shmaddr = NULL;

int pipefd1[2],pipefd2[2];
int filefd[2];

char curr_group = '1';

void unbuffered_printf(int fd,int outfd){
    int nbytes = 0;
    char buffer[100];
    while((nbytes=read(fd,buffer,100)) > 0){
        write(outfd,buffer,nbytes);
    }
    if(nbytes == -1){
        fprintf(stderr,"write error : %s\n",strerror(errno));
    }
}

void close_all_pipes(){
    close(pipefd1[0]);
    close(pipefd1[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);
}
void close_all_filefds(){
    close(filefd[0]);
    close(filefd[1]);
}

//increment no of interrupts
void increment_nkills(){
    NKILLS+=1;
    sprintf(shmaddr,"%d",NKILLS);
}

void toggle_group(){
    printf("toggling group\n");
    if(curr_group == '1'){
        printf("%d\n",GROUPID1);
        killpg(GROUPID1, SIGSTOP);
        killpg(GROUPID2, SIGCONT);
        curr_group = '2';
    }else{
        printf("%d\n",GROUPID2);
        killpg(GROUPID2, SIGSTOP);
        killpg(GROUPID1, SIGCONT);
        curr_group = '1';
    }
    increment_nkills();
    fflush(stdout);
}

//signal handler for SIGUSR1
static void sigusr1_handler(int signum){
    //D sent a signal
    toggle_group();
}


void ignore_signal(int signum){
    if(signal(signum,SIG_IGN) == SIG_ERR){
        fprintf(stderr, "ignore signal error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void default_signal(int signum){
    if(signal(signum,SIG_DFL) == SIG_ERR){
        fprintf(stderr, "default signal error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
}


//group1
void g1_task(char id){
    //child ignores SIGUSR1
    ignore_signal(SIGUSR1);
    close_all_filefds();
    printf("child g1 pid=%d, pgid=%d\n",getpid(),getpgid(getpid()));

    int fd = id=='1'?pipefd1[0]:pipefd2[0];
    unbuffered_printf(fd,STDOUT_FILENO);
    exit(EXIT_SUCCESS);
}


//group 2
void g2_task(char id){
    //child ignores SIGUSR1
    ignore_signal(SIGUSR1);
    //close_all_pipes();

    //initially, group1 is active
    raise(SIGSTOP);
    
    printf("child g2 pid=%d, pgid=%d\n",getpid(),getpgid(getpid()));
    
    int fd = id=='1'?pipefd1[0]:pipefd2[0];
    unbuffered_printf(fd,STDOUT_FILENO);
    exit(EXIT_SUCCESS);
}


void write_to_group(int fd){
    //write the received message to fds
    char buffer[1024];
    char initital_msg[40];
    int nbytes=0;
    int outfd1 = pipefd1[1];
    int outfd2 = pipefd2[1];
    int outfd3 = filefd[0];
    sprintf(initital_msg,"message from client %d to clild %d\n",fd,1);
    write(outfd1,initital_msg,strlen(initital_msg));
    sprintf(initital_msg,"message from client %d to clild %d\n",fd,2);
    write(outfd2,initital_msg,strlen(initital_msg));
    while((nbytes=read(fd,buffer,1024)) > 0){
        write(outfd1,buffer,nbytes);
        write(outfd2,buffer,nbytes);
        write(outfd3,buffer,nbytes);
    }
}

//parent
void parent_task(int gid1, int gid2){
    GROUPID1 = gid1;
    GROUPID2 = gid2;
    
    printf("%d\n",getpgid(getpid()));

    //create 2 fifos where C1,C2 can write
    if(mkfifo("./C1MESSAGE",IPC_CREAT | IPC_EXCL | 0666) == -1){
        if(errno!=EEXIST){
            fprintf(stderr,"fifo error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if(mkfifo("./C2MESSAGE",IPC_CREAT | IPC_EXCL | 0666) == -1){
        if(errno!=EEXIST){
            fprintf(stderr,"fifo error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }


    //shared memory for incrementing nkills
    int shmid;
    if((shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL)) == -1){
        if(errno != EEXIST){
            fprintf(stderr,"shmget error : %s",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    shmaddr = shmat(shmid,NULL,0);

    struct pollfd fds[2] = {
        {
            .fd=open("./C1MESSAGE",O_RDONLY | O_NONBLOCK),
            .events=POLLIN,
            .revents = 0
        },
        {
            .fd=open("./C2MESSAGE",O_RDONLY | O_NONBLOCK),
            .events=POLLIN,
            .revents = 0
        }
    };
    int status;

    //check if C1/C2 has sent message
    while(1){
        printf("listening...\n");
        fflush(stdout);

        status = poll(fds,2,-1);

        if(status == -1){
            fprintf(stderr,"poll error : %s\n",strerror(errno));
        }else if(status > 0){
            //message received, write to fds

            if(fds[0].revents & POLLIN){
                write_to_group(fds[0].fd);                
            }
            if(fds[0].revents & (POLLIN | POLLHUP | POLLERR)){
                close(fds[0].fd);
                fds[0].fd = open("./C1MESSAGE",O_RDONLY | O_NONBLOCK);
            }

            if(fds[1].revents & POLLIN){
                write_to_group(fds[1].fd);                
            }
            if(fds[1].revents & (POLLIN | POLLHUP | POLLERR)){
                close(fds[1].fd);
                fds[1].fd = open("./C2MESSAGE",O_RDONLY | O_NONBLOCK);
            }
        }
    }
}

int main(){
    /*
        SIGUSR1 is used for toggling
    */
    if(signal(SIGUSR1,sigusr1_handler) == SIG_ERR){
        fprintf(stderr, "signal error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server PID : %d\n",getpid());

    //create pipe
    if(pipe(pipefd1) == -1){
        printf("pipe error\n");
    }
    if(pipe(pipefd2) == -1){
        printf("pipe error\n");
    }

    //create fds
    filefd[0] = open("msg1.txt",O_RDONLY);
    filefd[1] = open("msg2.txt",O_WRONLY);


    //A1,A2 (Group 1)
    pid_t pid1,pid2;
    
    pid1 = fork();

    if(pid1 == 0){
        if(setpgid(getpid(),getpid()) == -1){
            fprintf(stderr, "setgid error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        };

        //group1 next process, pgid same as it's parent pid
        pid_t pid_child;
        pid_child = fork();
        if(pid_child == 0){
            g1_task('2');
        }else if(pid_child > 0){
            if(setpgid(pid_child,getpid()) == -1){
                fprintf(stderr, "setgid error : %s\n",strerror(errno));
                exit(EXIT_FAILURE);
            };
        }
        g1_task('1');
    }

    
    
    
    //B1,B2 (Group 2)
    pid2 = fork();
    if(pid2 == 0){
        if(setpgid(getpid(),getpid()) == -1){
            fprintf(stderr, "setgid error : %s\n",strerror(errno));
            exit(EXIT_FAILURE);
        };

        //group2 next process, pgid same as it's parent pid
        pid_t pid_child;
        pid_child = fork();
        if(pid_child == 0){
            g2_task('2');
        }else if(pid_child > 0){
            if(setpgid(pid_child,getpid()) == -1){
                fprintf(stderr, "setgid error : %s\n",strerror(errno));
                exit(EXIT_FAILURE);
            };
        }
        g2_task('1');
    }
    

    //parent listens for message in C1MESSAGE , C2MESSAGE
    parent_task(pid1,pid2);
}