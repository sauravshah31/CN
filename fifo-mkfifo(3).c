#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>



int main(int argc, char **argv){
    char *process_name;
    int process_no, nbytes;
    int status;
    pid_t pid;

    process_name = malloc(sizeof(char) * 10);

    if(argc==1){
        sprintf(process_name,"%d",0);
        process_no = 0;
    }else{
        process_name = strcpy(process_name,argv[1]);
        process_no = atoi(argv[1]);
    }

    if(argc != 1){
        //we have message from previous process
        //write output from previous process
        char buffer[1024];
        int fd = open("fifo",O_RDONLY);
        while((nbytes=read(fd,buffer,1024)) > 0){
            (void)write(STDOUT_FILENO,buffer,nbytes);
        }
        close(fd);
    }else{
        if(mkfifo("./fifo", 0666) == -1){
            if(errno != EEXIST ){
                fprintf(stderr, "mkfifo error : %s\n",strerror(errno));
                exit(EXIT_FAILURE);
            }
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
        //parent writes
        int fd = open("fifo",O_WRONLY);

        dprintf(fd, "Hi, you have message from %d\n",process_no);

        close(fd);
        pid = waitpid(pid, &status, 0);
        if(pid<0){
            fprintf(stderr, "wait error : %s\n",strerror(errno));
        }
    }else{
        sprintf(process_name,"%d",process_no+1);
        execlp(argv[0],argv[0],process_name,NULL);
    }
}