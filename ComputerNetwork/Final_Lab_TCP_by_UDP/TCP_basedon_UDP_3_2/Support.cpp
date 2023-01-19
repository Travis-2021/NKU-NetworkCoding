#include<winsock2.h>
#include<iostream>
#include<time.h>
#include"Support.h"
#include<thread>

using namespace std;

double RTO = 100;
bool RECIVE_ACK = 0;


void bindIPort(sockaddr_in *addr, u_short ProType, u_short port, const char *ip_str) {
    addr->sin_family = ProType;
    addr->sin_port = htons(port);  // 服务器端口
    if (ip_str == NULL) {
        addr->sin_addr.S_un.S_addr = INADDR_ANY;
    } else {
        addr->sin_addr.S_un.S_addr = inet_addr(ip_str);
    }
}

void Add_Checksum(char* buf) {
    HEADER* hdr = (HEADER*)buf;
    int TotalSize = sizeof(HEADER) + hdr->Size;     // Header+Data大小
    int TotalBlock = (TotalSize%2==0)? TotalSize/2:TotalSize/2+1; // 需要计算块的数量，向上取整
//    int TotalBlock = TotalSize/2; // 需要计算块的数量，向上取整
    WORD* data = (WORD*)buf; // 16位对齐
    DWORD temp = 0;
    for(int i=0;i<TotalBlock;i++){
        temp += data[i];
        if(temp > 0xFFFF){  // 产生进位，向后移位，加入计算
            temp += temp>>16;
            temp &= 0xFFFF;
        }
    }
    hdr->Checksum = (~temp)&0xFFFF; // 取低16位，取反
}

bool Test_Checksum(char* buf){


//    HEADER *hdr = (HEADER *) buf;
//    int TotalSize = sizeof(HEADER) + hdr->Size;     // Header+Data大小
//
//    int TotalBlock = (TotalSize % 2 == 0) ? TotalSize / 2 : TotalSize / 2 + 1; // 需要计算块的数量，向上取整
////    int TotalBlock = TotalSize/2; // 需要计算块的数量，向上取整
//    WORD *data = (WORD *) buf; // 16位对齐
//    DWORD temp = 0;
//
//    for (int i = 0; i < TotalBlock; i++) {
//        temp += data[i];
//        if (temp > 0xFFFF) {  // 产生进位，向后移位，加入计算
//            temp += temp >> 16;
//            temp &= 0xFFFF;
//        }
//    }
//
//    if (temp == 0xFFFF) {
//        return true;
//    } else {
//        return false;
//    }

    return true;
}

void reverse(u_char &x){
    x = (x==1)? 0:1;
}

void SetHeader(HEADER& hdr,u_short seq,u_short ack,u_char Flags,u_short Size){
    hdr.seq = seq;
    hdr.ack = ack;
    hdr.Flags = Flags;
    hdr.Size = Size;
    hdr.Checksum = 0;
}

void SetDESC(DESC& desc,u_long ByteNum,u_long PackNum,u_long WinNum,char* FileName){
    desc.ByteNum = ByteNum;
    desc.PackNum = PackNum;
    desc.WinNum = WinNum;
    memcpy(desc.FileName,FileName,sizeof(desc.FileName));
}

void PrintDESC(DESC& desc){
    cout<<"文件大小："<<desc.ByteNum<<"Bytes"<<endl;
    cout<<"需要发送包个数："<<desc.PackNum<<endl;
    cout<<"窗口大小："<<desc.WinNum<<endl;
    cout<<"文件名称："<<desc.FileName<<endl;
}

void TimerThread(SOCKET& s, sockaddr_in& addr, char *buf, int len){
    clock_t start=clock();
    while(!RECIVE_ACK){
        clock_t end=clock();
        if((end-start)>RTO) {
            /* 重新发送 */
            sendto(s,buf,len,0,(sockaddr*)&addr,sizeof(addr));
            cout<<"丢包，超时重传..."<<endl;
            start=clock();
        }
    }
}

