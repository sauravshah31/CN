#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define cmd "pidof "

int main(){
    char process_name[100];
    char command[120];
    printf("process name : ");
    scanf("%s",process_name);

    strcpy(command, cmd);
    strcpy(command+strlen(cmd), process_name);

    FILE* term = popen(command, "r");
    int pid;

    fscanf(term, "%d",&pid);

    int status = pclose(term);

    if(status == 1){
        printf("No process found\n");
    }else{
        printf("pid is : %d\n",pid);
    }

    return 0;
}