#include <stdio.h>

#include <unistd.h>

int main(){
    printf("pid : %d\n",getpid());
    pause();
    printf("...exiting\n");
}