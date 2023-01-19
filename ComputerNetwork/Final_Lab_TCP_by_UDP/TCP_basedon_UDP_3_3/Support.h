#ifndef NETWORKCODE_SUPPORT_H
#define NETWORKCODE_SUPPORT_H

#include<winsock2.h>
#include<time.h>
//#include "FileUtil.h"


#define FIN 0b1
#define SYN 0b10
#define ACK 0b100
#define END 0b1000
#define DataMaxSize 1024
#define WinSize 5
#define INIT_SIZE 7674

extern double RTO;
extern bool RECIVE_ACK;
extern int base;
extern int nextseqnum;
extern int sum;
extern int ReadSize;
extern int cwnd;        // 拥塞控制：窗口大小
extern int ssthresh;    // 拥塞控制：阈值


struct HEADER { // 10 Bytes
    u_short Checksum;
    u_short seq;
    u_short ack;
    u_char Flags;
    u_short Size;
};



struct DESC{    // 32 Bytes
    u_long ByteNum;
    u_long PackNum;
    u_long WinNum;
    char FileName[20];
};

struct Pkt{
    HEADER hdr;
    char data[DataMaxSize];
};

void SetHeader(HEADER& hdr,u_short seq,u_short ack,u_char Flags,u_short Size);

void SetDESC(DESC& desc,u_long ByteNum,u_long PackNum,u_long WinNum,char* FileName);

void PrintDESC(DESC& desc);

void PrintWndSst(int cwnd, int ssthresh);

void bindIPort(sockaddr_in *addr, u_short ProType, u_short port, const char *ip_str);

void Client_Connect(SOCKET &s, sockaddr_in &addr);

void Client_DisConnect(SOCKET &s, sockaddr_in &addr);

void Server_Connect(SOCKET &s, sockaddr_in &addr);

void Server_DisConnect(SOCKET &s, sockaddr_in &addr);

void Add_Checksum(char* buf);

bool Test_Checksum(char* buf);

void reverse(u_char &x);

void TimerThread(SOCKET& s, sockaddr_in& addr, char buf[], int len);

void ClientSendThread(SOCKET& s, sockaddr_in& addr);

void ClientRecvThread(SOCKET& s, sockaddr_in& addr);



#endif //NETWORKCODE_SUPPORT_H
