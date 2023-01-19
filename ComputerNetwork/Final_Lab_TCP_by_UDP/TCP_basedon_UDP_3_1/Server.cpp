#include <iostream>
#include <winsock2.h>
#include <fstream>
#include "Support.h"

using namespace std;



int main() {
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));  // 初始化为0

    bindIPort(&addr, AF_INET, 18000, "127.0.0.1");
    if (bind(s, (LPSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
        cout << "绑定IP、端口失败" << endl;

    Server_Connect(s, addr);

    HEADER hdr;
    HEADER ackhdr;
    char buf[DataMaxSize +sizeof(hdr)];
    char bufhdr[sizeof(hdr)];
    int addrlen = sizeof(addr);
    int sum = 0;

    // test.txt 1.jpg 2.jpg 3.jpg helloworld.txt
    char Filepath[] = "../recvfile/1.jpg";

    ofstream outFile(Filepath,ios::out|ios::binary|ios::app);

    SetHeader(ackhdr,0,0,ACK,0);
    while(true){
        recvfrom(s,buf,sizeof(buf),0,(sockaddr*)&addr,&addrlen);
        memcpy(&hdr,buf,sizeof(hdr));
        if(Test_Checksum(buf)){
            sum += hdr.Size;
            cout<<"单个数据报包大小："<<hdr.Size<<"Bytes"<<endl;
            cout<<"已成功接收"<<sum<<"Bytes..."<<endl;
            ackhdr.ack = hdr.seq;
            memcpy(bufhdr,&ackhdr,sizeof(ackhdr));
            Add_Checksum(bufhdr);
            sendto(s,bufhdr,sizeof(bufhdr),0,(sockaddr*)&addr,sizeof(addr));
            outFile.write(buf+sizeof(hdr),hdr.Size);
        }
        if(hdr.Flags == END){
            break;
        }
    }

    Server_DisConnect(s,addr);
    WSACleanup();
}
