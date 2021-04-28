#include <stdio.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/ip.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(){
    int rsfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

    const int max_bits = pow(2,16)-1;

    unsigned char *packet = (unsigned char *) malloc(max_bits);
    memset(packet, 0, max_bits);
    struct sockaddr addr;
    int addr_len = sizeof(addr);

    int buflen = recvfrom(rsfd, packet, max_bits, 0, &addr, (socklen_t *)&addr_len);

    if(buflen < 0){
       perror("receive()");
       return -1;
    }

    struct ethhdr *eth = (struct ethhdr *) packet;

    printf("\n...Ethernet header...\n");
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("\t|-Protocol : %d\n",eth->h_proto);

    unsigned short iphdrlen;
    struct iphdr *ip = (struct iphdr*)(packet + sizeof(struct ethhdr));

    struct sockaddr_in source,dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    printf("\n...IP header...\n");
    printf("\t-Version : %d\n", (unsigned int)ip->version);
    printf("\t-Internet Header Length : %d DWORDS\n",(unsigned int)ip->ihl);
    printf("\t-Type of Service : %d\n",(unsigned int)ip->tos);
    printf("\t-Total Length : %d bytes\n",ntohs(ip->tot_len));
    printf("\t-Identification : %d \n",ntohs(ip->id));
    printf("\t-Time to Live : %d \n",(unsigned int)ip->ttl);
    printf("\t-Protocol : %d\n",(unsigned int)ip->protocol);    
    printf("\t-Header checksum : %d\n",ntohs(ip->check));
    printf("\t-Source IP : %s\n", inet_ntoa(source.sin_addr));
    printf("\t-Destination IP : %s\n", inet_ntoa(dest.sin_addr));



    close(rsfd);

}