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

#include <arpa/inet.h>

struct psd_udp{
    struct in_addr src;
    struct in_addr dst;
    unsigned char pad;
    unsigned char proto;
    unsigned short udp_len;
    struct udphdr udp;
};

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

unsigned short in_cksum_udp(int src, int dst, unsigned short *addr, int len){
    struct psd_udp buf;

    memset(&buf, 0, sizeof(buf));
    buf.src.s_addr = src;
    buf.dst.s_addr = dst;
    buf.pad = 0;
    buf.proto = IPPROTO_UDP;
    buf.udp_len = htons(len);
    memcpy(&(buf.udp), addr, len);

    return in_cksum((unsigned short *)&buf, 12 + len);
}

int main(int argc ,char **argv){
    struct ip ip;
    struct udphdr udp;
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
    ip.ip_p = IPPROTO_UDP; //upper layer protocol layer
    ip.ip_sum = 0x0;    //checksum
    ip.ip_src.s_addr = inet_addr("10.0.2.15");
    ip.ip_dst.s_addr = inet_addr("10.0.2.15");

    ip.ip_sum = in_cksum((unsigned short *)&ip, sizeof(ip));

    memcpy(packet, &ip, sizeof(ip));

    //udp data
    udp.uh_sport = htons(5000); //source port
    udp.uh_dport = htons(6000);    //destination port
    udp.uh_ulen = htons(8); //udp header length + udp data
    udp.uh_sum = 0;         //udp checksum

    udp.uh_sum = in_cksum_udp(ip.ip_src.s_addr, ip.ip_dst.s_addr, (unsigned short *) &udp, sizeof(udp));

    memcpy(packet + 20, &udp, sizeof(udp));

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