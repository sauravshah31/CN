#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

int isclosed = 0;


int main(){
    int rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

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
            struct sockaddr_in source_ip, dest_ip;

            memset(&source_ip, 0, sizeof(source_ip));
            memset(&dest_ip, 0, sizeof(dest_ip));
            source_ip.sin_addr.s_addr = ip_header->daddr;
            dest_ip.sin_addr.s_addr = ip_header->saddr;

            printf("PACKET RECEIVED...\n");
            printf("\t|-Source IP\t:%s\n",inet_ntoa(source_ip.sin_addr));
            printf("\t|-Destination IP\t:%s\n",inet_ntoa(dest_ip.sin_addr));
            printf("\t|-Source port\t:%u\n",ntohs(tcp_header->source));
            printf("\t|-Destination port\t:%u\n",ntohs(tcp_header->dest));
            printf("\n");

            
        }
    }

    close(rsfd);

}