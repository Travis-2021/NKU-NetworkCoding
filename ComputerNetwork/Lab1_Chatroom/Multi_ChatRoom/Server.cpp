#include<iostream>
#include<WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;


DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);

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

    //if (s == INVALID_SOCKET)
    //{
    //	cout << "socket error:" << WSAGetLastError() << endl;
    //	return 0;
    //}

    // 2.绑定端口和ip
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));  // 初始化为0

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);  // 服务器端口
    // addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器ip  老函数不使用了
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int len = sizeof(sockaddr_in);
    bind(s, (SOCKADDR*)&addr, len);

    // 3.监听
    listen(s, 5);


    // 主线程循环接收客户端的连接
    while (true)
    {
        sockaddr_in addrClient;
        len = sizeof(sockaddr_in);
        // 4.接受成功返回与client通讯的Socket
        SOCKET c = accept(s, (SOCKADDR*)&addrClient, &len);
        if (c != INVALID_SOCKET)
        {
            // 创建线程，并且传入与client通讯的套接字
            HANDLE hThread = CreateThread(NULL, 0, ThreadFun, (LPVOID)c, 0, NULL);
            CloseHandle(hThread); // 关闭对线程的引用
        }
    }

    // 6.关闭监听套接字
    closesocket(s);

    // 清理winsock2的环境
    WSACleanup();



    return 0;
}


DWORD WINAPI ThreadFun(LPVOID lpThreadParameter)
{
    // 5.与客户端通讯，发送/接收数据
    SOCKET c = (SOCKET)lpThreadParameter;

    cout << "欢迎" << c << "进入聊天室！" << endl;

    // 发送数据
    char buf[100] = "进入聊天室";
    // sprintf(buf, "欢迎 %d 进入聊天室！", c);
    send(c, buf, 100, 0);

    // 循环接收客户端数据
    int ret = 0;
    do
    {
        char buf2[100] = { 0 };
        ret = recv(c, buf2, 100, 0);

        cout << c << "说：" << buf2 << endl;

    } while (ret != 0 && ret != SOCKET_ERROR);

    cout << c << "离开了！" << endl;

    return 0;
}

