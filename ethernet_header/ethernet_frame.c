#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netinet/if_ether.h>


#include <net/ethernet.h>




#define ADAPTER "enp0s3"



//https://en.wikipedia.org/wiki/Address_Resolution_Protocol
struct arp_hdr{
    unsigned short int htype;   /*Hardware Type*/
    unsigned short int ptype;   /*Protocol Type*/
    unsigned char haddr_len;    /*Hardware Address length*/
    unsigned char paddr_len;    /*Protocol Address length*/
    unsigned short int op;      /*Operation*/
    unsigned char shaddr[6];    /*Sender Hardware Address*/
    unsigned char spaddr[4];    /*Sender Protocol Address*/
    unsigned char thaddr[6];    /*Target Hardware address*/
    unsigned char tpaddr[4];    /*Target Protocol Address*/
}__attribute__((packed));

int rsfd;

void clean(int ext){
    close(rsfd);
    if(ext)
        exit(0);
}


int main(){
    rsfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(rsfd < 0){
        perror("socket()");
        clean(1);
    }
    if(setsockopt(rsfd, SOL_SOCKET, SO_BINDTODEVICE, ADAPTER, strlen(ADAPTER)+1) < 0){
        perror("setsockopt()");
        clean(1);
    }

    size_t  data_size;
    const size_t max_len = pow(2,16)-1;
    char buf[max_len];

    signal(SIGINT, clean);
    while (1)
    {
        data_size = recvfrom(rsfd, buf, max_len, 0, NULL, NULL);
        if(data_size < 0){
            perror("recvfrom()");
            continue;
        }

        struct ethhdr *eth = (struct ethhdr *)buf;
        struct arp_hdr *arp = (struct arp_hdr*) (buf+sizeof(struct ethhdr));



        if(htons(eth->h_proto) == ETH_P_ARP){
            printf("---ETHERNET HEADER WITH ARP PAYLOAD RECEIVED---\n");
            printf("\tSource MAC Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
                    eth->h_source[0],
                    eth->h_source[1],
                    eth->h_source[2],
                    eth->h_source[3],
                    eth->h_source[4],
                    eth->h_source[5]);
            printf("\tDestination MAC Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
                    eth->h_dest[0],
                    eth->h_dest[1],
                    eth->h_dest[2],
                    eth->h_dest[3],
                    eth->h_dest[4],
                    eth->h_dest[5]);
            printf("---ARP HEADER---\n");
            printf("\tHardware Type : %u\n",arp->htype);
            printf("\tProtocol Type : %u\n",arp->ptype);
            printf("\tHardware address length : %u bytes\n",arp->haddr_len*8);
            printf("\tProtocol address length : %u bytes\n",arp->paddr_len*8);
            printf("\tOperation : %u\n",arp->op);
            printf("\tSender MAC Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
                    arp->shaddr[0],
                    arp->shaddr[1],
                    arp->shaddr[2],
                    arp->shaddr[3],
                    arp->shaddr[4],
                    arp->shaddr[5]);
            printf("\tSender Protocol Address : %u.%u.%u.%u\n",
                    arp->spaddr[0],
                    arp->spaddr[1],
                    arp->spaddr[2],
                    arp->spaddr[3]);
            printf("\tDestination MAC Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
                    arp->thaddr[0],
                    arp->thaddr[1],
                    arp->thaddr[2],
                    arp->thaddr[3],
                    arp->thaddr[4],
                    arp->thaddr[5]);
            printf("\tDestination Protocol Address : %u.%u.%u.%u\n",
                    arp->tpaddr[0],
                    arp->tpaddr[1],
                    arp->tpaddr[2],
                    arp->tpaddr[3]);   
        }

    }
    


}