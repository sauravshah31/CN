#include <stdio.h>

#include <pcap/pcap.h>

#define NETWORK_ADAPTER "enp0s3"


int main(){
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
}