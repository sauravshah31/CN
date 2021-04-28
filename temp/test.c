#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t p;

    printf("PID : %d\n",getpid());
    printf("PROGRAM 1\n");

    char *a[]={"./test1",NULL};
    
    execvp(a[0],a);

    return 0;

    
}