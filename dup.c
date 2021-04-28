
#include <stdio.h>
#include <unistd.h>

int main(){
    int fd;
    int nbytes;
    char buffer[50];

    int stdin_fd_copy = dup(0);
    int stdout_fd_copy = dup(1);

    FILE * stdout_copy = fdopen(stdout_fd_copy,"w");
    fprintf(stdout_copy,"Press ctrl+d to stop\n");

    while((nbytes=read(stdin_fd_copy, buffer, 50))>0){
        write(stdout_fd_copy,buffer,nbytes);
    }
    

    fprintf(stdout_copy, "---Exiting---\n");
    (void)fclose(stdout_copy);
    return -1;
}