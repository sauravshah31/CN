#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/uio.h>

int main(){
    //ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
    /*
            struct iovec {
               void  *iov_base;     //Starting address 
               size_t iov_len;      //Number of bytes to transfer
            };
    */
    ssize_t nbytes;
    char *s1=(char *)malloc(sizeof(char) * 10),*s2 = (char *)malloc(sizeof(char) * 12);
    struct iovec iov[2] = {
        {
            .iov_base=s1,
            .iov_len=10-1
        },
        {
            .iov_base=s2,
            .iov_len=12-1
        }
    };
    int fd = open("./input.txt",O_RDONLY);
    nbytes = readv(fd, iov, 2);

    printf("%s\n",s1);
    printf("%s\n",s2);

    close(fd);

}
