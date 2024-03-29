#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>


unsigned short in_cksum(unsigned short *addr, int len){
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1){
        sum += *w++;
        nleft -= 2;
    }

    if(nleft == 1){
        *(unsigned char *) (&answer) = *(unsigned char*) w;
        sum += answer;
    }

    sum = (sum>>16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

int main(int argc, char **argv){
    struct ip ip;
    struct udphdr udp;
    struct icmp icmp;
    int sfd;
    const int on = 1;
    struct sockaddr_in sin;
    u_char *packet;

    packet = (u_char *)malloc(60);

    //layer II (IP) protocol data
    ip.ip_hl = 0x5;     //20 bytes
    ip.ip_v = 0x4;      //IPv4
    ip.ip_tos = 0x0;    //packet precedence
    ip.ip_len = htons(60);  //total length 
    ip.ip_id = htons(12375); //id
    ip.ip_off = 0x0;    //fragment offset
    ip.ip_ttl = 64;     //time to live, max hops that packet can pass
    ip.ip_p = IPPROTO_ICMP; //upper layer protocol layer
    ip.ip_sum = 0x0;    //checksum
    ip.ip_src.s_addr = inet_addr("10.0.2.15");
    ip.ip_dst.s_addr = inet_addr("10.0.2.15");

    ip.ip_sum = in_cksum((unsigned short *)&ip, sizeof(ip));

    memcpy(packet, &ip, sizeof(ip));

    //layer III (ICMP) data
    icmp.icmp_type = ICMP_ECHO; //type
    icmp.icmp_code = 0; //code 0 for echo request
    icmp.icmp_id = 1234; //id
    icmp.icmp_seq = 0;  //sequence number
    icmp.icmp_cksum = 0; //check sum

    icmp.icmp_cksum = in_cksum((unsigned short *)&icmp, 8);

    memcpy(packet + 20, &icmp, 8);

    //inject into network
    if( (sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0){
        perror("raw socket");
        exit(1);
    }

    //tell kernel that ip header are applied
    if(setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt");
        exit(1);
    }

    //destination for kernel
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip.ip_dst.s_addr;

    if(sendto(sfd, packet, 60, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0){
        perror("sendto");
        exit(1);
    }

    return 0;

}