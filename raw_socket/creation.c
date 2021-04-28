#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


unsigned short checksum(unsigned short *addr, int len){
    
}

int main(){
    int rsfd;
    rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    const int on = 1;
    if(setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt()");
        return -1;
    }

    

}