#ifndef NETWORKCODE_SUPPORT_H
#define NETWORKCODE_SUPPORT_H

#include<winsock2.h>
#include<time.h>

#define FIN 0b1
#define SYN 0b10
#define ACK 0b100
#define END 0b1000
#define DataMaxSize 1024

extern double RTO;
extern bool RECIVE_ACK;


struct HEADER {
    u_short Checksum;
    u_char seq;
    u_char ack;
    u_char Flags;
    u_short Size;
};

void SetHeader(HEADER& hdr,u_char seq,u_char ack,u_char Flags,u_short Size);

void bindIPort(sockaddr_in *addr, u_short ProType, u_short port, const char *ip_str);

void Client_Connect(SOCKET &s, sockaddr_in &addr);

void Client_DisConnect(SOCKET &s, sockaddr_in &addr);

void Server_Connect(SOCKET &s, sockaddr_in &addr);

void Server_DisConnect(SOCKET &s, sockaddr_in &addr);

void Add_Checksum(char* buf);

bool Test_Checksum(char* buf);

void reverse(u_char &x);

void TimerThread(SOCKET& s, sockaddr_in& addr, char buf[], int len);

#endif //NETWORKCODE_SUPPORT_H
