#include<iostream>
#include"SupportFunc.h"
#include<Winsock2.h>
#include<ctime>

using namespace std;

void ReceiveThread(SOCKET c) {
    int ret;
    do {
        // 接收消息
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        ret = recv(c, buf, 1024, 0);
        getNowtime();
        cout << "Client：" << buf << endl;
    } while (ret != 0 && ret != SOCKET_ERROR);

    cout << "断开连接" << endl;
}

void SendThread(SOCKET c) {
    int ret;
    do {
        // 发送消息
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        cout << "输入发送内容：";
        cin >> buf;
        getNowtime();
        cout << "Server：" << buf << endl;
        ret = send(c, buf, 1024, 0);
    } while (ret != 0 && ret != SOCKET_ERROR);

    cout << "断开连接" << endl;
}

void getNowtime() {
    // 基于当前系统的当前日期/时间
    tm now{};
    __time64_t CurrentTime;
    _time64(&CurrentTime);
    _localtime64_s(&now, &CurrentTime);

    // 输出 tm 结构的各个组成部分
    cout << 1900 + now.tm_year << ".";
    cout << 1 + now.tm_mon << ".";
    cout << now.tm_mday;
    cout << "  " << now.tm_hour << ":";
    cout << now.tm_min << ":";
    cout << now.tm_sec << endl;

}