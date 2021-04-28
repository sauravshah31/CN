#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t p;

    printf("PID : %d\n",getpid());
    printf("PROGRAM 2\n");

    char *a[]={"./test2",NULL};
    
    execvp(a[0],a);

    return 0;

    
}