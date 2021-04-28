#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    int rsfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

    struct ifreq ifreq_i;

    //get network interface index
    memset(&ifreq_i, 0, sizeof(ifreq_i));
    strncpy(ifreq_i.ifr_name, "enp0s3", 6);

    if(ioctl(rsfd, SIOCGIFINDEX, &ifreq_i) < 0){
        perror("ioctl() interface index");
        return -1;
    }
    
    printf("network interface index : %d\n", ifreq_i.ifr_ifindex);


    //getting mac address of network interface
    struct ifreq ifreq_c;
    memset(&ifreq_i, 0, sizeof(ifreq_c));
    strncpy(ifreq_c.ifr_name, "enp0s3", 6);

    if(ioctl(rsfd, SIOCGIFHWADDR, &ifreq_c) < 0){
        perror("ioctl() mac address");
        return -1;
    }

    printf("MAC address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",ifreq_c.ifr_hwaddr.sa_data[0],ifreq_c.ifr_hwaddr.sa_data[1],ifreq_c.ifr_hwaddr.sa_data[2],ifreq_c.ifr_hwaddr.sa_data[3],ifreq_c.ifr_hwaddr.sa_data[4],ifreq_c.ifr_hwaddr.sa_data[5]);


    //getting ip address of interface
    struct ifreq ifreq_ip;
    memset(&ifreq_ip, 0, sizeof(ifreq_ip));
    strncpy(ifreq_ip.ifr_name, "enp0s3", 6);

    if(ioctl(rsfd, SIOCGIFADDR, &ifreq_ip) < 0){
        perror("ioctl() ip address");
        return -1;
    }
    printf("IP address : %d.%d.%d.%d\n",ifreq_ip.ifr_addr.sa_data[0], ifreq_ip.ifr_addr.sa_data[1], ifreq_ip.ifr_addr.sa_data[2], ifreq_ip.ifr_addr.sa_data[3]);


    //constructing Ethernet header
    unsigned char *sendbuff = (unsigned char*) malloc(64);
    memset(sendbuff, 0, 64);

    struct ethhdr *eth = (struct ethhdr *)(sendbuff);
    eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
    eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
    eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
    eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
    eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
    eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

    eth->h_dest[0] = DESTMAC0;

}