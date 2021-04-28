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
#include <pcap/pcap.h>

#define NETWORK_ADAPTER "enp0s3"
#define SRC_ADDR        "0.0.0.0"
#define DST_ADDR        "0.0.0.0"
#define SRC_PORT        5000
#define DST_PORT        6000


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



char* create_dns_query_packet(void){
    size_t nbytes;
    char *buf, *ptr;


    const size_t hdr_s = sizeof(struct ip) + sizeof(struct udphdr);
    buf = malloc(hdr_s + 100);



    ptr = buf + hdr_s;

    *((uint16_t *) ptr) = htons(1234);
    ptr += 2;
    *((uint16_t *) ptr) = htons(0x0100);
    ptr += 2;
    *((uint16_t *) ptr) = htons(1);
    ptr += 2;
    *((uint16_t *) ptr) = 0;
    ptr += 2;
    *((uint16_t *) ptr) = 0;
    ptr += 2;

    memcpy(ptr , "\001a\014root-servers\003net\000", 20);
    ptr += 20;
    *((uint16_t *) ptr) = htons(1);
    ptr += 2;
    *((uint16_t *) ptr) = htons(1);
    ptr += 2;

    nbytes = (ptr - buf) - hdr_s;


    //fill ip,udp header
    struct ip *ip;
    struct udphdr *udp;

    ip = (struct ip *) buf;

    //IP Header
    ip->ip_hl = 0x5;     //20 bytes
    ip->ip_v = 0x4;      //IPv4
    ip->ip_tos = 0x0;    //packet precedence
    ip->ip_len = htons(60);  //total length 
    ip->ip_id = htons(12375); //id
    ip->ip_off = 0x0;    //fragment offset
    ip->ip_ttl = 64;     //time to live, max hops that packet can pass
    ip->ip_p = IPPROTO_UDP; //upper layer protocol layer
    ip->ip_sum = 0x0;    //checksum
    ip->ip_src.s_addr = inet_addr(SRC_ADDR);
    ip->ip_dst.s_addr = inet_addr(DST_ADDR);

    ip->ip_sum = in_cksum((unsigned short *)ip, sizeof(struct ip));


    //udp header
    udp->uh_sport = htons(SRC_PORT); //source port
    udp->uh_dport = htons(DST_PORT);    //destination port
    udp->uh_ulen = htons(8); //udp header length + udp data
    udp->uh_sum = 0;         //udp checksum

    udp->uh_sum = in_cksum_udp(ip->ip_src.s_addr, ip->ip_dst.s_addr, (unsigned short *) udp, sizeof(struct udphdr));

}

void send_dns_query_packet(int rsfd, char *buf){
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ((struct ip*)buf)->ip_dst.s_addr;

    if(sendto(rsfd, buf, sizeof(buf), 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0){
        perror("sendto");
        exit(1);
    }

}

int main(){
    /*
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t * handle;
    if((handle = pcap_create(NETWORK_ADAPTER, errbuf)) == NULL){
        fprintf(stderr,"create error : %s\n",errbuf);
        return -1;
    }

    if(pcap_activate(handle) != 0){
        fprintf(stderr,"activate error : %s\n",pcap_geterr(handle));
        return -1;
    }

    pcap_close(handle);
    */

    int rsfd = socket(PF_PACKET, SOCK_RAW, 0);
    const int on = 1;

    if(rsfd < 0){
        perror("raw socket");
        exit(1);
    }

    if(setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt");
        exit(1);
    }

    char *buf = create_dns_query_packet();
    send_dns_query_packet(rsfd, buf);
}