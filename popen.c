#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>


int main(int argc, char **argv){
    char *process_name;
    char *command;
    int process_no, nbytes;

    process_name = malloc(sizeof(char) * 10);

    if(argc==1){
        snprintf(process_name,10,"%d",0);
        process_no = 0;
    }else{
        process_name = strcpy(process_name,argv[1]);
        process_no = atoi(argv[1]);
    }

    if(argc != 1){
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

    process_no++;

    command = malloc(sizeof(char)*50);
    sprintf(command,"%s %d",argv[0],process_no);
    
    
    FILE *fp = popen(command,"w");
    if(fp == NULL){
        fprintf(stderr,"popen error : %s",strerror(errno));
        return -1;
    }

    fprintf(fp,"Hi, you have message from %d\n",process_no);
    fclose(fp);
    return 0;   
}