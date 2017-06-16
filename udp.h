#ifndef UDP_H
#define UDP_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>


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


unsigned short CheckSum(unsigned short *buffer, int size);
size_t buildUDP(int rawfd, int srcport, int destport, char* buffer, char* srcaddr, char* destaddr);


#endif