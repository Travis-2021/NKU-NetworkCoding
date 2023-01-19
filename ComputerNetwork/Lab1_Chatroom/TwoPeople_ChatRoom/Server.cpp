#include<iostream>
#include<WinSock2.h>
#include<Ws2tcpip.h>
#include<thread>
#include "SupportFunc.h"

using namespace std;

int main()
{
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
    {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    // 1.创建套接字
    // socket(ip4/ip6, 流式/数据报式，协议)
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET)
    {
        cout << "socket error:" << WSAGetLastError() << endl;
        return 0;
    }

    // 2.绑定端口和ip
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));  // 初始化为0

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);  // 服务器端口
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器ip  老函数不使用了
    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int len = sizeof(sockaddr_in);
    bind(s, (SOCKADDR*)&addr, len);

    // 3.监听
    listen(s, 5);

    // 4.接受成功返回与client通讯的Socket
    sockaddr_in addrClient;
    len = sizeof(sockaddr_in);
    SOCKET c = accept(s, (SOCKADDR*)&addrClient, &len);
    if (c != INVALID_SOCKET)
    {
        // 创建线程，并且传入与client通讯的套接字
        cout<<"成功建立连接"<<endl;
        thread rec(ReceiveThread, c);
        thread sed(SendThread, c);

        rec.join();
        sed.join();
    }

    // 5.关闭监听套接字
    closesocket(s);

    // 6.清理winsock2的环境
    WSACleanup();

    return 0;
}

