#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    int stdin_fd_copy = dup(STDIN_FILENO); //for undoing swapping at last

    //use out.txt as stdin
    int fd = open("./out.txt",O_RDONLY);

    //duplicate fd(out.txt) to act as stdin(STDIN_FILENO=0)
    int new_fd = dup2(fd,STDIN_FILENO);

    int nbytes=0;
    char buffer[50];
    while((nbytes=read(STDIN_FILENO,buffer,50))>0){
        write(STDOUT_FILENO,buffer,nbytes);
    }

    //undo the stdin dup
    (void)dup2(STDIN_FILENO, stdin_fd_copy);

    return 0;
}