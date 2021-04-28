#include <stdio.h>

#include <pcap/pcap.h>


int main(){
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevsp;

    if(pcap_findalldevs(&alldevsp, errbuf) != 0){
        fprintf(stderr,"findalldev error : %s",errbuf);
        return -1;
    }

    pcap_if_t *tmp = alldevsp;
    printf("Devices List:\n");
    while(tmp->next != NULL){
        printf("\t%s : %s\n",tmp->name, tmp->description);
        tmp = tmp->next;
    }

    pcap_freealldevs(alldevsp);

    return 0;
}