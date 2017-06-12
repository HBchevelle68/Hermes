#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>  // struct icmp, ICMP_ECHO

#include "udp.h"

void usage();




int main(int argc, char* argv[]){

    int raw_sock, pcktcount;
    size_t tot_len;
    struct sockaddr_in sin;

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
    if ((raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() raw socket creation failed ");
        exit(EXIT_FAILURE);
    }

    char dgram[4096];
    memset(dgram, 0, sizeof(dgram));
    
    tot_len = buildUDP(raw_sock, 1, 1, dgram);


    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr (argv[1]);

    printf("Sending...\n");

    for(int count = 1; count <= pcktcount; count++){
        if(sendto(raw_sock, dgram, tot_len, 0, (struct sockaddr*) &sin, sizeof(sin)) < 0){
            perror("sendto() failed");
            exit(-1);
        }
        else {
            printf("Sent UDP dgram #%d \n", count);
        }
        //sleep(1);
    }

    close(raw_sock);
    
    return 0;
}



void usage(){
    printf("Usage: sudo ./hermes [SRC ADDR] [DEST ADDR] -c [NUM]\n\n");
    printf("\t[DEST ADDR] \tIPv4 address of target\n");
    printf("\t[NUM] \tNumber of packets to send\n");
}
