#include <stdio.h>

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


void check_child(pid_t pid){
    int stat = kill(pid, SIGUSR2);
    if(stat == -1){
        printf("%d exited\n",pid);
    }

}
int main(){
    pid_t pids[3];

    for(int i=0;i<3;i++){
        pids[i] = fork();
        if(pids[i] == 0){
            if(execl("./child","./child",NULL) < 0){
                printf("error : %s\n",strerror(errno));
            }
        }
    }

    while(1){
        printf("...checking\n");
        for(int i=0;i<3;i++){
            check_child(pids[i]);
        }
        sleep(3);
    }
}