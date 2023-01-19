#include <iostream>
#include <winsock2.h>
#include <fstream>
#include "Support.h"
#include "ClientFunc.h"
#include <thread>
using namespace std;



int main() {
    /* --------------------------------准备-----------------------------------------*/
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    bindIPort(&addr, AF_INET, 16000, "127.0.0.1");

    Client_Connect(s, addr);
    SendFile = true;
    /* --------------------------------开启多线程-----------------------------------------*/

    thread Timer(ClientTimerThread,ref(s),ref(addr));
    thread Recv(ClientRecvThread,ref(s),ref(addr));
    thread Send(ClientSendThread,ref(s),ref(addr));
    Timer.join();
    Recv.join();
    Send.join();









    //Client_DisConnect(s,addr);
    WSACleanup();
}







