#include <iostream>
#include <winsock2.h>
#include <fstream>
#include "Support.h"
#include "ServerFunc.h"

using namespace std;



int main() {
    /* --------------------------------׼��-----------------------------------------*/
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));  // ��ʼ��Ϊ0

    bindIPort(&addr, AF_INET, 18000, "127.0.0.1");
    if (bind(s, (LPSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
        cout << "��IP���˿�ʧ��..." << endl;

    Server_Connect(s, addr);
    /* --------------------------------�����ļ�������Ϣ-----------------------------------------*/
    DESC desc;
    char descbuf[sizeof(DESC)];
    int addrlen = sizeof(addr);
    char Filepath[] = "../recvfile/";
    recvfrom(s, descbuf, sizeof(descbuf), 0, (sockaddr *) &addr, &addrlen);
    memcpy(&desc,descbuf,sizeof(desc));
    PrintDESC(desc);
    strcat(Filepath,desc.FileName);
    cout<<"�ļ�·����"<<Filepath<<endl;
    ofstream outFile(Filepath,ios::out|ios::binary|ios::app);
    /* --------------------------------����-----------------------------------------*/
    HEADER hdr;
    HEADER ackhdr;
    char buf[DataMaxSize +sizeof(HEADER)];
    char bufhdr[sizeof(HEADER)];
    int sum = 0;
    int Expectseq = 0;

    SetHeader(ackhdr,0,0,ACK,0);
    while(true){
        recvfrom(s,buf,sizeof(buf),0,(sockaddr*)&addr,&addrlen);
        memcpy(&hdr,buf,sizeof(hdr));
        if(hdr.Size==INIT_SIZE)
            continue;
        if(Test_Checksum(buf)&&hdr.seq==Expectseq){
            Expectseq++;
            memcpy(&hdr,buf,sizeof(hdr));

            sum += hdr.Size;
            cout<<"���ݰ�"<<hdr.seq<<"��С��"<<hdr.Size<<"Bytes\t";
            cout<<"�ѳɹ�����"<<sum<<"Bytes..."<<endl;

            // ���á�����ACK�ذ�
            ackhdr.ack = hdr.seq;
            memcpy(bufhdr,&ackhdr,sizeof(ackhdr));
            Add_Checksum(bufhdr);
            sendto(s,bufhdr,sizeof(bufhdr),0,(sockaddr*)&addr,sizeof(addr));
            cout<<"����ȷ�Ϻ�"<<ackhdr.ack<<endl;
            // д�ļ�
            outFile.write(buf+sizeof(hdr),hdr.Size);
        }
        if(Expectseq==desc.PackNum){
            break;
        }
    }









    /*-------------------------------------------------------------------*/
    //Server_DisConnect(s,addr);
    WSACleanup();
}
