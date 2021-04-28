#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/uio.h>

int main(){
    //ssize_t write(int fd, const struct iovec *iov, int iovcnt);
    /*
            struct iovec {
               void  *iov_base;     //Starting address 
               size_t iov_len;      //Number of bytes to transfer
            };
    */
    ssize_t nbytes=0;
    char *s1="This is message 1\n",*s2 = "This is message 2";
    struct iovec iov[2] = {
        {
            .iov_base=s1,
            .iov_len=strlen(s1)
        },
        {
            .iov_base=s2,
            .iov_len=strlen(s2)
        }
    };
    int fd = open("./input.txt", O_WRONLY | O_CREAT| O_EXCL ,0666);
    if(fd ==-1 && errno == EEXIST){
        printf("already exists\n");
        fd = open("./input.txt", O_WRONLY);
    }
    nbytes = writev(fd, iov, 2);
    close(fd);
}
