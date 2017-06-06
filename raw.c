#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 

#include <sys/socket.h>
#include <netinet/tcp.h>	//tcp header
#include <netinet/ip.h>		//ip header
#include <sys/ioctl.h>		//sys calls
#include <net/if.h>   
#include <unistd.h>

int main(int argc, char* argv[]){

	int raw_sock;
	struct ifreq ifr;


	char* interface = malloc(40);
	memset(interface,0,sizeof(40));
	strcpy(interface, "ens33");

	// Submit request for a socket descriptor to look up interface.
	if ((raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket() raw socket creation failed ");
		exit(EXIT_FAILURE);
	}



	close(raw_sock);
	

	
	return 0;
}