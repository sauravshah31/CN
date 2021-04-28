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
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include <signal.h>

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>

using namespace std;


#define IPPROTO_EP  255

#define NET_INTERFACE       "enp0s3"



void get_network_interface_ip_address(int rsfd, char dest_arr[]){
    struct ifreq ifreq_ip;
    memset(&ifreq_ip, 0, sizeof(ifreq_ip));
    strncpy(ifreq_ip.ifr_name, NET_INTERFACE, strlen(NET_INTERFACE));

    if(ioctl(rsfd, SIOCGIFADDR, &ifreq_ip) < 0){
        perror("ioctl() ip address");
        exit(-1);
    }

    sprintf(dest_arr, "%d.%d.%d.%d", ifreq_ip.ifr_addr.sa_data[0], ifreq_ip.ifr_addr.sa_data[1], ifreq_ip.ifr_addr.sa_data[2], ifreq_ip.ifr_addr.sa_data[3]);

    printf("Interface IP is %s\n",dest_arr);
}

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


void respond_client_enquiry(char *service_name,int rsfd,int port, char sip_addr[], char dip_addr[] ){
    struct sockaddr_in addr;
    struct ip ip;
    u_char *packet;

    //construct the IP header
    packet = (u_char *)malloc(60);
    //prepare json response
    char json[1025];
    sprintf(json, "{\"name\":\"%s\",\"port\":\"%d\"}", service_name, port);
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
    memcpy(packet + 20, &json, strlen(json));


    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip.ip_dst.s_addr;

    if(sendto(rsfd, packet, 60, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0){
        perror("sendto() enquire");
        exit(-1);
    }

    printf("enquiry sent\n");
}

void handle_client(char *service_name, int sfd){
    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    int nsfd = accept(sfd, (struct sockaddr*) &caddr,&caddrlen);
    pid_t pid = fork();

    if(pid == 0){
        close(sfd);
        signal(SIGPIPE,SIG_IGN);
        char msg[1024];
        sprintf(msg,"service %s ack",service_name);
        send(nsfd, msg, strlen(msg),0);
        close(nsfd);
        signal(SIGPIPE,SIG_DFL);
        exit(-1);
    }else if(pid > 0){
        close(nsfd);
    }

}

int main(int argc, char *argv[]){

    if(argc != 3){
        fprintf(stderr,"Usage: %s port_no service_name\n",argv[0]);
        return -1;
    }

    const int backlog = 5;
    const uint16_t port_number = atoi(argv[1]);

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd<0){
        printf("socket error : %sn",strerror(errno));
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_number);

    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        printf("bind err : %s\n",strerror(errno));
        return -1;
    }
    if(listen(sfd,backlog) < 0){
        printf("listen error : %s\n",strerror(errno));
        return -1;
    }

    int rsfd;
    rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_EP);
    if(rsfd < 0){
        perror("socket()");
        return -1;
    }

    const int on = 1;
    if(setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt()");
        return -1;
    }

    char sip_addr[17], dip_addr[17];
    get_network_interface_ip_address(rsfd, sip_addr);
    //using same machine, so same ip
    strncpy(dip_addr, sip_addr, 16);



    //listen to sockets
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(rsfd,&rfds);
    FD_SET(sfd,&rfds);

    while(1){
        fd_set ofd = rfds;

        int retval = select(rsfd+1,&rfds,NULL,NULL,NULL);

        if(retval > 0){
            if(FD_ISSET(rsfd, &rfds)){
                respond_client_enquiry(argv[2],rsfd, port_number, sip_addr, dip_addr);
            }
            if(FD_ISSET(sfd, &rfds)){
                handle_client(argv[2], sfd);
            }
        }

        rfds = ofd;
    }


    
    close(rsfd);
    close(sfd);    

}