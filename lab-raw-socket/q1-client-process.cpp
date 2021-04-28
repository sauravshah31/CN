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


#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>

using namespace std;


#define IPPROTO_EP  255

#define NET_INTERFACE       "enp0s3"

unordered_set<string> required_services;



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


void send_enquiry(int rsfd, char sip_addr[], char dip_addr[]){
    struct sockaddr_in addr;
    struct ip ip;
    u_char *packet;

    //construct the IP header
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


    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip.ip_dst.s_addr;

    if(sendto(rsfd, packet, 60, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0){
        perror("sendto() enquire");
        exit(-1);
    }

    printf("enquiry sent\n");
}


unordered_map<string,string> parse_json(char *buf){
    //1 level nested json parser
    int i=1;
    unordered_map<string,string> json;
    while(buf[i] != '}' && buf[i] != '\0'){
        i += 1;
        string key="",value="";
        while(buf[i] != '"' && buf[i] != '\0')
            key += buf[i++];

        i += 3;

        while(buf[i] != '"' && buf[i] != '\0')
            value += buf[i++];

        json[key] = value;

        i += 2;
    }

    return json;
}


void avail_service(char *ip, int port, string service_name){
    //create a child process to get the service

    pid_t pid = fork();

    if(pid == 0){
        //establish a tcp connection
        const uint16_t port_number = port;

        int sfd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip);
        addr.sin_port = htons(port_number);

        connect(sfd,(struct sockaddr*) &addr, sizeof(addr));


        cout<<"connected to service : "<<service_name<<endl;
        

        char buf[1024];

        while(recv(sfd, buf, 1024, 0) > 0){
            printf("%s\n",buf);    
        }
        close(sfd);
        exit(0);
    }
}

void listen_for_service_response(int rsfd){
    const int max_servicel = 20;
    char buffer[max_servicel];
    memset(buffer, 0, max_servicel);
    struct sockaddr addr;
    if(recvfrom(rsfd, buffer, max_servicel, 0, &addr,(socklen_t *) sizeof(addr)) < 0){
        perror("recvfrom()");
    }


    char ip[17];
    sprintf(ip,"%d.%d.%d.%d",addr.sa_data[0], addr.sa_data[1], addr.sa_data[2], addr.sa_data[3]);
    printf("%d.%d.%d.%d",addr.sa_data[0], addr.sa_data[1], addr.sa_data[2], addr.sa_data[3]);
    printf("data : %s",buffer);

    auto json = parse_json(buffer);
    string service_name = json["name"];
    int port = stoi(json["port"]);

    //check if service is requested

    if(required_services.find(service_name) != required_services.end()){
        avail_service(ip, port, service_name);
    }
}

int main(int argc, char *argv[]){

    if(argc <2){
        fprintf(stderr,"Usage: %s service1 service2 ...\n",argv[0]);
        return -1;
    }

    
    for(int i=1;i<argc;i++){
        required_services.insert(string(argv[i]));
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


    
    

    //send enquiry request
    send_enquiry(rsfd, sip_addr, dip_addr);


    //listen for response
    listen_for_service_response(rsfd);

    
    close(rsfd);
    

}