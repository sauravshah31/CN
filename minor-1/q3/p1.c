
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(){
    int p1,p2,p3,p4,p5,p6;
    int org_stdin = dup(STDIN_FILENO);
    int org_stdout = dup(STDOUT_FILENO);

    //p2,p3,p4 -> stdout as stdin of p1
    dup2(STDOUT_FILENO, STDIN_FILENO);

    //create all the processes
    p1 = getpid();
    p2 = fork();
    if(p2 == 0){
        //parent (p1)
        p3 = fork();
        if(p3 == 0){
            //parent (p1)
            p4 = fork();
            if(p4 == 0){
                //parent (p1)
                p5 = fork();
                if(p5 == 0){
                    p6 = fork();
                    if(p6 == 0){
                        
                    }else{
                        execlp("./writer","./writer",NULL);
                    }
                    int status;
                    waitpid(p5, &status, 0);

                    //p5 exists, stdout of p1 -> stdin p5;
                    dup2(dup(org_stdout),dup(org_stdin));
                }else{
                    //stdout of p1 -> stdin of p5
                    dup2(dup(org_stdin),dup(org_stdout));

                    //p5 is writer
                    execlp("./writer","./writer",NULL);

                    
                }
            }else{
                //p4 is reader
                execlp("./reader","./reader",NULL);
            }
        }else{
            //p3 is reader
            execlp("./reader","./reader",NULL);
        }
    }else{
        //p2 is reader
        execlp("./reader","./reader",NULL);
    }

    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
}