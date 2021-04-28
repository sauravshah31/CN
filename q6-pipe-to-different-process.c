/*
    Pipe the process of one process into a different process
    P1 -> P2 -> P3
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv){
    char *process_name;
    pid_t pid;
    int status;
    int nbytes;
    int org_stdin_fd = -1;
    int process_no;

    process_name = malloc(sizeof(char) * 10);

    if(argc==1){
        snprintf(process_name,sizeof(process_name),"%d",0);
        process_no = 0;
    }else{
        process_name = strcpy(process_name,argv[1]);
        process_no = atoi(argv[1]);
    }

    if(STDIN_FILENO != 0){
        //we have message from previous process
        //write output from previous process
        char buffer[1024];
        while((nbytes=read(STDIN_FILENO,buffer,1024)) > 0){
            (void)write(STDOUT_FILENO,buffer,nbytes);
        }
    }

    if(process_no > 5){
        return 0;
    }

    
    

    pid = fork();
    if(pid==-1){
        fprintf(stderr, "fork error : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(pid > 0){
        //parent
    
        //wrtie message for next process
        if(argc == 1){
            int pipefd[2];
            status = pipe(pipefd);
            if(status == -1){
                fprintf(stderr, "pipe error : %s\n",strerror(errno));
                exit(EXIT_FAILURE);
            }

            //copy actaul stdin_number 
            org_stdin_fd = dup(STDIN_FILENO);
            (void)dup2(STDIN_FILENO, pipefd[0]);

            (void)close(pipefd[0]);
            (void)write(pipefd[1],"Hi, you have message from ",28);
            (void)write(pipefd[1],process_name,strlen(process_name));
            (void)write(pipefd[1],"\n",1);
            (void)close(pipefd[1]);
        }else{
            (void)write(STDIN_FILENO,"Hi, you have message from ",28);
            (void)write(STDIN_FILENO,process_name,strlen(process_name));
            (void)write(STDIN_FILENO,"\n",1);
            (void)close(STDIN_FILENO);
        }

        

        pid = waitpid(pid, &status, 0);
        if(pid<0){
            fprintf(stderr, "wait error : %s\n",strerror(errno));
        }

        if(org_stdin_fd!=-1){
            (void)dup2(STDIN_FILENO, org_stdin_fd);
        }
    }else{
        //child
        snprintf(process_name,sizeof(process_name),"%d",process_no+1);
        execlp(argv[0],argv[0],process_name,NULL);
    }
    
    return 0;
}