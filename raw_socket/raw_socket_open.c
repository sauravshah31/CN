#include <stdio.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>


int main(){
    int rsfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if(rsfd < 0){
        perror("socket()");
        return -1;
    }
}