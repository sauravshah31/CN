#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>

#define ISS_LOC     "/tmp/iss_sock.socket"

int isclosed = 0;


int main(){

    int rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

    unlink(ISS_LOC);

    int iss_usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(iss_usfd < 0){
        perror("socket()");
        return -1;
    }

    struct sockaddr_un iss_addr;
    iss_addr.sun_family = AF_UNIX;
    strcpy(iss_addr.sun_path, ISS_LOC);
    if(connect(iss_usfd, (const struct sockaddr *) &iss_addr, sizeof(struct sockaddr)) < 0){
        perror("connect() to ISS");
        return -1;
    }
    printf("linked to ISS...\n");

    const int max_bits = pow(2,16)-1;

    unsigned char *packet = (unsigned char *) malloc(max_bits);
    memset(packet, 0, max_bits);
    struct sockaddr addr;
    int addr_len = sizeof(addr);

    while(!isclosed){
        int buflen = recvfrom(rsfd, packet, max_bits, 0, &addr, (socklen_t *)&addr_len);
        if(buflen < 0){
            perror("receive()");
        }else{
            struct iphdr *ip_header = (struct iphdr*) packet;
            struct tcphdr *tcp_header = (struct tcphdr*) (packet+ ip_header->ihl*4);

            const uint16_t port_number = tcp_header->dest;
            //send to Super Server

            char data[5];
            sprintf(data, "%u", port_number);
            if(write(iss_usfd, data, 5) < 0){
                perror("send() to iss");
            }
        }
    }

    close(rsfd);

}