//
// Created by lenovo on 2022/10/9.
//
#include<iostream>
#include<winsock2.h>
#include <Ws2tcpip.h>
#include<thread>
#include"SupportFunc.h"
using namespace std;




int main()
{
    WSADATA wd;
    WSAStartup(MAKEWORD(2, 2), &wd);

    // 1.创建套接字
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 2.连接服务器
    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int len = sizeof(sockaddr_in);
    if (connect(s, (SOCKADDR*)&addr, len) == SOCKET_ERROR)
    {
        cout << "connect  error：" << GetLastError() << endl;
        return 0;
    }
    cout << "成功建立连接" << endl;

    // 3.随时发送数据给服务器
    thread rec(ReceiveThread, s);
    thread sed(SendThread, s);

    rec.join();
    sed.join();


    // 4.关闭套接字
    closesocket(s);

    // 5.清除winsock2环境
    WSACleanup();


    return 0;
}


