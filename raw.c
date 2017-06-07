#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 

#include <sys/socket.h>
#include <netinet/tcp.h>	//tcp header
#include <netinet/udp.h>
#include <netinet/ip.h>		//ip header
#include <sys/ioctl.h>		//sys calls
#include <arpa/inet.h>
#include <net/if.h>   
#include <unistd.h>

#define UDP_PROTO 16

unsigned short csum (unsigned short *buf, int nwords);

int main(int argc, char* argv[]){

	int raw_sock, opt = 1;
	const int* op = &opt;

	struct ip* iph;
	struct udphdr* udp;
	struct sockaddr_in sin;


	// Submit request for a socket descriptor to look up interface.
	if ((raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
		perror("socket() raw socket creation failed ");
		exit(EXIT_FAILURE);
	}

	char dgram[4096];
	memset(dgram, 0, sizeof(dgram));

	iph = (struct ip*) dgram;
	udp = (struct udphdr*) dgram + sizeof(struct ip);


	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	sin.sin_addr.s_addr = inet_addr("8.8.8.8");


	iph->ip_hl = 5;
	iph->ip_v = 4;
	iph->ip_tos = 16;
	iph->ip_len = sizeof (struct ip) + sizeof (struct udphdr);	/* no payload */
	iph->ip_id = htonl (54321);	/* the value doesn't matter here */
	iph->ip_off = 0;
	iph->ip_ttl = 255;
	iph->ip_p = UDP_PROTO;
	iph->ip_sum = 0;		/* set it to 0 before computing the actual checksum later */
	iph->ip_src.s_addr = inet_addr ("192.168.175.131");
	iph->ip_dst.s_addr = inet_addr ("8.8.8.8");

	udp->uh_sport = htons(12345);
	udp->uh_dport = htons(53);
	udp->uh_ulen = htons(sizeof(struct udphdr));
	udp->uh_sum = 0; //auto handled by kernel

	iph->ip_sum = csum((unsigned short*)dgram, sizeof(struct ip) + sizeof(struct udphdr));

	if(setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, op, sizeof(opt)) < 0){
		perror("setsockopt() error");
		exit(-1);
	}
	else
		printf("setsockopt() is OK.\n");

	printf("Trying...\n");
	printf("Using raw socket and UDP protocol\n");

	for(int count = 0; count <= 20; count++){
		if(sendto(raw_sock, dgram, iph->ip_len, 0, (struct sockaddr*) &sin, sizeof(sin)) < 0){
			perror("sendto() failed");
			exit(-1);
		}
		else {
			printf("UDP Count #%d \n", count);
		}
		sleep(1);
	}

	close(raw_sock);
	
	return 0;
}

unsigned short csum (unsigned short *buf, int nwords) {
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}