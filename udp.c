#include "udp.h"


unsigned short CheckSum(unsigned short *buffer, int size){
    unsigned long cksum=0;
    while(size >1){
        cksum+=*buffer++;
        size -=sizeof(unsigned short);
    }
    if(size){
        cksum += *(unsigned char*)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (unsigned short)(~cksum);
}


size_t buildUDP(int rawfd, int srcport, int destport, char* buffer, char* srcaddr, char* destaddr){

	char* temp_csum;
    struct iphdr* iph;
    struct udphdr* udph;
    struct udpchk uchk;

	iph = (struct iphdr*) buffer;
    udph = (struct udphdr*) (buffer + sizeof(struct iphdr));

    char* data = buffer + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(data , "Wooo it worked!");

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    iph->id = htonl (54321); 
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;      
    iph->saddr = inet_addr (srcaddr); 
    iph->daddr = inet_addr (destaddr);
    
    //Fill in the IP Header
    udph->source = htons (srcport);
    udph->dest = htons (destport);
    udph->len = htons(sizeof(struct udphdr) + strlen(data)); 
    udph->check = 0;

    //IP checksum
    iph->check = CheckSum((unsigned short*)buffer, iph->tot_len);
    
    //begin udp checksum
    uchk.source_address = iph->saddr;
    uchk.dest_address = iph->daddr;
    uchk.placeholder = 0;
    uchk.protocol = IPPROTO_UDP;
    uchk.udp_length = htons(sizeof(struct udphdr) + strlen(data));

    //size of IP header, UDP header, and data
    int size = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    //malloc mem 
    temp_csum = malloc(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));

    //copy the specific data from udpchk structure 
    memcpy(temp_csum, (char*) &uchk, sizeof(struct udpchk));
    //copy the UDP header after udpchk structure 
    memcpy(temp_csum + sizeof(struct udpchk), udph, sizeof(struct udphdr) + strlen(data));
    //compute checksum and store to UDP headers
    udph->check = CheckSum((unsigned short*) temp_csum, size);

    free(temp_csum);

    return iph->tot_len;
}

