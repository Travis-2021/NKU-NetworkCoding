#include <iostream>
#include <winsock2.h>
#include <fstream>
#include "Support.h"
#include "FileUtil.h"
#include <thread>
using namespace std;



int main() {
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr;
    char buf[DataMaxSize+sizeof(HEADER)];
    char hdrbuf[sizeof(HEADER)];

    memset(&addr, 0, sizeof(sockaddr_in));
    bindIPort(&addr, AF_INET, 16000, "127.0.0.1");

    Client_Connect(s, addr);

    // test.txt 1.jpg 2.jpg 3.jpg helloworld.txt
    char Filepath[] = "../test/1.jpg";

    ifstream inFile(Filepath,ios::in|ios::binary);
    if(!inFile.is_open()){
        cout<<"打开文件失败"<<endl;
        inFile.close();
    }

    int len = File::fileSize(Filepath);
    cout<<"文件大小："<<len<<"Bytes"<<endl;
    int num = len/DataMaxSize+1;
    cout<<"需要发送包个数："<<num<<endl;

    int ReadSize,sum=0,i=1;
    HEADER hdr;
    u_char current_seq = 0;
    int addrlen = sizeof(addr);

    clock_t start,end;
    start = clock();
    while(true) {
        ReadSize = (len-sum)<DataMaxSize? (len-sum):DataMaxSize;
        sum += ReadSize;
        SetHeader(hdr,current_seq,0,0,ReadSize);
        if(i==num){
            hdr.Flags = END;
        }

        // 向缓冲区写数据
        memcpy(buf,&hdr,sizeof(hdr));
        inFile.read(buf+sizeof(hdr),ReadSize);
        Add_Checksum(buf);

        // 发送数据包
        sendto(s,buf,ReadSize+sizeof(hdr),0,(sockaddr*)&addr,sizeof(addr));
        RECIVE_ACK = FALSE;

        // 启动计时器
        thread Timer(TimerThread, ref(s),ref(addr),buf, ReadSize+sizeof(hdr));
        Timer.detach();

        // 收到ACK
        recvfrom(s, hdrbuf, sizeof(hdrbuf), 0, (sockaddr *) &addr, &addrlen);
        RECIVE_ACK = true;
        memcpy(&hdr,hdrbuf,sizeof(hdrbuf));

        // 确认是ACK
        if((hdr.Flags==ACK)&&(hdr.ack==current_seq)&& Test_Checksum(buf)){
            reverse(current_seq);
            cout<<"已成功发送"<<sum<<"Bytes..."<<endl;
        }

        i++;
        // 最后一包发送完毕
        if(i>num) {
            break;
        }
    }
    end = clock();
    cout<<"总传输时间："<<double(end-start)/1000<<"s"<<endl;
    cout<<"平均吞吐率："<<double(len)/(end-start)<<"Kb/s"<<endl;

    Client_DisConnect(s,addr);

    WSACleanup();
}







