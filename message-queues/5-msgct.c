/*
#include <linux/types.h>
#include <linux/ipc.h>
#include <linux/msg.h>
*/
#include <syscall.h>
//#include "/usr/include/linux/ipc.h"
#define KEY 1
#define MSG_SIZE 1024


int main(){

    long msgid;
    msgid = sys_msgget(KEY, 0);
    //ipc(MSGCTL, KEY, 0, IPC_STAT, &qinfo, 0);

}