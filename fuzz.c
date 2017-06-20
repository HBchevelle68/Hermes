#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

/*
    The udp checksum is only performed on certain data
    and the UDP head. This stuct contains the extra
    data required for the checksum
*/
struct udpchk {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};

void usage();
unsigned short CheckSum(unsigned short *buffer, int size);
size_t buildUDP(int rawfd, int srcport, int destport, char* buffer, char* srcaddr, char* destaddr);

int main(int argc, char* argv[]){

    int rsfd, pcktcount;
    const int on = 1;
    size_t tot_len;
    char* name = "ens33";
    struct sockaddr_in sin;
    //struct sockaddr_ll if_addr;

    if(argc < 3) {
        usage();
        return 0;
    }
    
    if(argv[3][0] == '-' && argv[3][1] == 'c'){
        pcktcount = atoi(argv[4]);
    } 
    else {
        usage();
        return 0;
    }

    //create raw socket
    if ((rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() raw socket creation failed ");
        exit(EXIT_FAILURE);
    }

    //grab interface index
    struct ifreq interface;
    memset(&interface, 0, sizeof(struct ifreq));
    strncpy((char*)interface.ifr_name, name, IF_NAMESIZE);
    if(ioctl(rsfd, SIOCGIFINDEX, &interface) < 0){
        perror("IF index ioctl error: ");
        exit(EXIT_FAILURE);
    }

    // Set flag so socket expects IPv4 header.
    if (setsockopt (rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror ("setsockopt() failed to set IP_HDRINCL ");
        exit (EXIT_FAILURE);
    }

    // Bind socket to interface index.
    if (setsockopt (rsfd, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface)) < 0) {
        perror ("setsockopt() failed to bind to interface ");
        exit (EXIT_FAILURE);
    }

    char dgram[4096];
    memset(dgram, 0, sizeof(dgram));
    tot_len = buildUDP(rsfd, 1, 1, dgram, argv[1], argv[2]);


    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    printf("Sending...\n");

    for(int count = 1; count <= pcktcount; count++){
        if(sendto(rsfd, dgram, tot_len, 0, (struct sockaddr*) &sin, sizeof(sin)) < 0){
            perror("sendto() failed");
            exit(-1);
        }
        else {
            printf("Sent UDP dgram #%d \n", count);
        }
        //sleep(1);
    }

    close(rsfd);
    
    return 0;
}



void usage(){
    printf("Usage: sudo ./fuzz [SRC ADDR] [DEST ADDR] -c [NUM]\n\n");
    printf("\t[DEST ADDR] \tIPv4 address of target\n");
    printf("\t[NUM] \tNumber of packets to send\n");
}

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