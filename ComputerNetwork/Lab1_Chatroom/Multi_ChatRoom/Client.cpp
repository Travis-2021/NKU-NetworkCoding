#include<iostream>
#include<winsock2.h>
#include <Ws2tcpip.h>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

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
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int len = sizeof(sockaddr_in);
    connect(s, (SOCKADDR*)&addr, len);


    // 3.接收服务器消息
    char buf[100] = { 0 };
    recv(s, buf, 100, 0);
    cout << buf << endl;

    // 4.随时发送数据给服务器
    int ret = 0;
    do
    {
        char buf2[100] = { 0 };
        cout << "输入发送内容：" << endl;
        cin >> buf2;
        ret = send(s, buf2, 100, 0);
    } while (ret != 0 && ret != SOCKET_ERROR);

    // 5.关闭套接字
    closesocket(s);

    // 6.清除winsock2环境
    WSACleanup();


    return 0;
}


