#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

/*
	The udp checksum is only performed on certain data
	This stuct contains the data required
*/

struct udpchk {
	u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};


unsigned short csum (unsigned short *buf, int nwords);
unsigned short CheckSum(unsigned short *buffer, int size);

int main(int argc, char* argv[]){

	int raw_sock;// opt = 1;
	//const int* op = &opt;

	struct iphdr* iph;
	struct udphdr* udph;
	struct udpchk uchk;
	struct sockaddr_in sin;
	char* temp_csum;

	// Submit request for a socket descriptor to look up interface.
	if ((raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket() raw socket creation failed ");
		exit(EXIT_FAILURE);
	}

	char dgram[4096];
	memset(dgram, 0, sizeof(dgram));
	

	iph = (struct iphdr*) dgram;

	udph = (struct udphdr*) (dgram + sizeof(struct iphdr));

	char* data = dgram + sizeof(struct iphdr) + sizeof(struct udphdr);
	strcpy(data , "Wooo it worked!");

	sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("8.8.8.8");
     
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
    iph->saddr = inet_addr ("192.168.100.100"); 
    iph->daddr = sin.sin_addr.s_addr;

	udph->source = htons (6666);
    udph->dest = htons (8622);
    udph->len = htons(sizeof(struct udphdr) + strlen(data)); 
    udph->check = 0;

	iph->check = CheckSum((unsigned short*)dgram, iph->tot_len);
	
	//begin udp checksum
	uchk.source_address = iph->saddr;
    uchk.dest_address = sin.sin_addr.s_addr;
    uchk.placeholder = 0;
    uchk.protocol = IPPROTO_UDP;
    uchk.udp_length = htons(sizeof(struct udphdr) + strlen(data) );

    int s = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    temp_csum = malloc(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));

    memcpy(temp_csum, (char*) &uchk, sizeof(struct udpchk));
    memcpy(temp_csum + sizeof(struct udpchk), udph, sizeof(struct udphdr) + strlen(data));

    udph->check = CheckSum((unsigned short*) temp_csum, s);

	printf("Trying...\n");
	printf("Using raw socket and UDP protocol\n");

	for(int count = 0; count <= 20; count++){
		if(sendto(raw_sock, dgram, iph->tot_len, 0, (struct sockaddr*) &sin, sizeof(sin)) < 0){
			perror("sendto() failed");
			exit(-1);
		}
		else {
			printf("UDP Count #%d \n", count);
		}
		//sleep(1);
	}

	close(raw_sock);
	
	return 0;
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
