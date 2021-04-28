#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>



unsigned short checksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}
	
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

	


int main(int argc, char *argv[]){

    if(argc !=2){
        fprintf(stderr,"Usage: %s protocol",argv[0]);
        return -1;
    }

    int rsfd;
    rsfd = socket(AF_INET, SOCK_RAW, atoi(argv[1]));
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

    const int on = 1;
    if(setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt()");
        return -1;
    }

    while(1){
        char sip_addr[16], dip_addr[16];
        printf("Enter spoffed source IP : ");
        scanf("%s",sip_addr);
        printf("Enter destination IP : ");
        scanf("%s",dip_addr);
        const int max_msg_len = pow(2,16)-1;
        struct sockaddr_in addr;


        struct ip ip;
        struct udphdr udp;
        struct icmp icmp;
        u_char *packet;

        packet = (u_char *)malloc(60);
        ip.ip_hl = 0x5;
        ip.ip_v = 0x4;
        ip.ip_tos = 0x0;
        ip.ip_len = htons(60);
        ip.ip_id = htons(getpid());
        ip.ip_off = 0x0;
        ip.ip_ttl = 250;
        ip.ip_p = IPPROTO_ICMP;
        ip.ip_dst.s_addr = inet_addr(sip_addr);
        ip.ip_dst.s_addr = inet_addr(dip_addr);
        ip.ip_sum = checksum((unsigned short *)&ip, sizeof(ip));

        memcpy(packet, &ip, sizeof(ip));

        icmp.icmp_type = ICMP_ECHO;
        icmp.icmp_code = 0;
        icmp.icmp_id = 1234;
        icmp.icmp_seq = 0;
        icmp.icmp_cksum = 0;
        icmp.icmp_cksum = checksum((unsigned short *) &icmp, 8);

        memcpy(packet+20, &icmp, 8);

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = ip.ip_dst.s_addr;

        if(sendto(rsfd, packet, 60, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0){
            perror("sendto()");
        }
    }

}