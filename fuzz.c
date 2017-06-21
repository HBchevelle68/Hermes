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
#include <netdb.h>

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

int main(int argc, char* argv[]){

    int rsfd, pcktcount;
    const int on = 1;
    char* temp_csum;
    char* name = "ens33";
    struct sockaddr_in *ipv4, sin;
    struct ip* iph;
    struct udphdr* udph;
    struct udpchk uchk;
    struct addrinfo hints, *res;
    void* tmp;

    char* dst_ip = malloc(INET_ADDRSTRLEN);

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
    snprintf (interface.ifr_name, sizeof (interface.ifr_name), "%s", name);
    if(ioctl(rsfd, SIOCGIFINDEX, &interface) < 0){
        perror("IF index ioctl error: ");
        exit(EXIT_FAILURE);
    }
    close(rsfd);

    // Fill out hints for getaddrinfo().
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = hints.ai_flags | AI_CANONNAME;

    // Resolve target using getaddrinfo().
    if (getaddrinfo (argv[2], NULL, &hints, &res) != 0) {
        perror("getaddrinfo() error");
        exit (EXIT_FAILURE);
    }
    printf("here\n");
    ipv4 = (struct sockaddr_in *) res->ai_addr;
    tmp = &(ipv4->sin_addr);
    if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
         perror("inet)_ntop() error");
        exit (EXIT_FAILURE);
    }
    freeaddrinfo (res);

    printf("here1\n");

    char dgram[IP_MAXPACKET];
    memset(dgram, 0, sizeof(dgram));
    
    iph = (struct ip*) dgram;
    udph = (struct udphdr*) (dgram + sizeof(struct ip));

    char* data = dgram + sizeof(struct ip) + sizeof(struct udphdr);
    strcpy(data , "Wooo it worked!");

    //Fill in the IP Header
    iph->ip_hl = 5;
    iph->ip_v = IPVERSION;
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + strlen(data);
    iph->ip_id = htonl(0); 
    iph->ip_off = 0;
    iph->ip_ttl = MAXTTL;
    iph->ip_p = IPPROTO_UDP;
    iph->ip_sum = 0;
    if(inet_pton(AF_INET, argv[1], &(iph->ip_src)) != 1){
        perror("ip_src inet_pton() error: ");
        exit(EXIT_FAILURE);
    }
    if(inet_pton(AF_INET, dst_ip, &(iph->ip_dst)) != 1){
        perror("ip_src inet_pton() error: ");
        exit(EXIT_FAILURE);
    }
    //IP header checksum
    iph->ip_sum = CheckSum((unsigned short*)&iph, 20);
    
    //Fill in the IP Header
    udph->source = htons(5555);
    udph->dest = htons(5555);
    udph->len = htons(sizeof(struct udphdr) + strlen(data)); 
    udph->check = 0;

    //begin udp checksum
    uchk.source_address = iph->ip_src.s_addr;
    uchk.dest_address = iph->ip_dst.s_addr;
    uchk.placeholder = 0;
    uchk.protocol = IPPROTO_UDP;
    uchk.udp_length = htons(sizeof(struct udphdr) + strlen(data));

    //size of IP header, UDP header, and data
    int size = sizeof(struct ip) + sizeof(struct udphdr) + strlen(data);
    //malloc mem 
    temp_csum = malloc(sizeof(struct ip) + sizeof(struct udphdr) + strlen(data));

    //copy the specific data from udpchk structure 
    memcpy(temp_csum, (char*) &uchk, sizeof(struct udpchk));
    //copy the UDP header after udpchk structure 
    memcpy(temp_csum + sizeof(struct udpchk), udph, sizeof(struct udphdr) + strlen(data));
    //compute checksum and store to UDP headers
    udph->check = CheckSum((unsigned short*) temp_csum, size);

    free(temp_csum);

    //create raw socket
    if ((rsfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() raw socket creation failed ");
        exit(EXIT_FAILURE);
    }

    // Set flag so socket expects IPv4 header.
    if (setsockopt (rsfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror ("setsockopt() failed to set IP_HDRINCL ");
        exit(EXIT_FAILURE);
    }

    // Bind socket to interface index.
    if (setsockopt (rsfd, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface)) < 0) {
        perror ("setsockopt() failed to bind to interface ");
        exit(EXIT_FAILURE);
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    printf("Sending...\n");

    for(int count = 1; count <= pcktcount; count++){
        if(sendto(rsfd, dgram, size, 0, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0){
            perror("sendto() failed");
            exit(EXIT_FAILURE);
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
