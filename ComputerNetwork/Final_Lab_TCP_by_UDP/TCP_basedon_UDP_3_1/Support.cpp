#include<winsock2.h>
#include<iostream>
#include<time.h>
#include"Support.h"
#include<thread>

using namespace std;

double RTO = 50;
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
    WORD* data = (WORD*)buf; // 16位对齐
    DWORD temp = 0;
    for(int i=0;i<TotalBlock;i++){
        temp += data[i];
        if(temp > 0xFFFF){  // 产生进位，向后移位，加入计算
            temp += temp>>0xFFFF;
            temp &= 0xFFFF;
        }
    }
    hdr->Checksum = (~temp)&0xFFFF; // 取低16位，取反
}

bool Test_Checksum(char* buf){
    HEADER *hdr = (HEADER *) buf;
    int TotalSize = sizeof(HEADER) + hdr->Size;     // Header+Data大小
    int TotalBlock = (TotalSize % 2 == 0) ? TotalSize / 2 : TotalSize / 2 + 1; // 需要计算块的数量，向上取整
    WORD *data = (WORD *) buf; // 16位对齐
    DWORD temp = 0;
    for (int i = 0; i < TotalBlock; i++) {
        temp += data[i];
        if (temp > 0xFFFF) {  // 产生进位，向后移位，加入计算
            temp += temp >> 0xFFFF;
            temp &= 0xFFFF;
        }
    }
    if (temp == 0xFFFF) {
        return true;
    } else {
        return false;
    }
}

void reverse(u_char &x){
    x = (x==1)? 0:1;
}

void SetHeader(HEADER& hdr,u_char seq,u_char ack,u_char Flags,u_short Size){
    hdr.seq = seq;
    hdr.ack = ack;
    hdr.Flags = Flags;
    hdr.Size = Size;
    hdr.Checksum = 0;
}

void Client_Connect(SOCKET &s, sockaddr_in &addr) {

    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);
    //clock_t start,end;

    /* -----------------------------------CLOSED----------------------------------- */
    SetHeader(hdr,0,0,SYN,0);
    memcpy(buf, &hdr, sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "握手1发送失败..." << endl;
        return;
    }
    // 启动计时器
    thread Timer1(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
    Timer1.detach();
    //start = clock();
    cout << "握手1发送成功..." << endl;

    /* -----------------------------------SYN_SENT----------------------------------- */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"握手2接收失败..."<<endl;
        return;
    }

    //end = clock();
    //cout<<"RTT:"<<(end-start)<<endl; //RTT == 3;

    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(SYN|ACK))&&(hdr.ack==0)){
        cout<<"握手2接收成功..."<<endl;
    }else{
        cout<<"握手2接收失败..."<<endl;
        return;
    }

    /* ---------------------------------ESTABLISHED----------------------------------- */
    // 发送第3次握手
    SetHeader(hdr,1,0,ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "握手3发送失败..." << endl;
        return;
    }
    // 启动计时器
    thread Timer2(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
    Timer2.detach();
    cout << "握手3发送成功..." << endl;
    cout << "客户端成功建立连接..." << endl;

    return;
}

void Client_DisConnect(SOCKET &s, sockaddr_in &addr)
{
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ESTABLISHED状态 */
    SetHeader(hdr,0,0,FIN|ACK,0);
    memcpy(buf, &hdr, sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "挥手1失败..." << endl;
        return;
    }
    cout<<"挥手1发送成功..."<<endl;

//    // 启动计时器
//    thread Timer3(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer3.detach();

    /* FIN_WAIT1 */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"挥手2接收失败..."<<endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(ACK))&&(hdr.ack==0)){
        cout<<"挥手2接收成功..."<<endl;
    }else{
        cout<<"挥手2接收失败..."<<endl;
        return;
    }

    /* FIN_WAIT2 */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"挥手3接收失败..."<<endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(FIN|ACK))){
        cout<<"挥手3接收成功..."<<endl;
    }else{
        cout<<"挥手3接收失败..."<<endl;
        return;
    }

    /* TIME_WAIT状态 */
    SetHeader(hdr,1,0,ACK,0);
    memcpy(buf, &hdr, sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "挥手4失败..." << endl;
        return;
    }
//    // 启动计时器
//    thread Timer4(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer4.detach();

    cout<<"挥手4发送成功..."<<endl;
    cout<<"客户端断开连接..."<<endl;
}

void Server_Connect(SOCKET &s, sockaddr_in &addr) {
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ------------------------------------LISTEN---------------------------------- */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "握手1接收失败..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == SYN)&&(hdr.seq==0)) {
        cout << "握手1接收成功..." << endl;
    } else {
        cout << "握手1接收失败..." << endl;
        return;
    }

    /* ----------------------------------SYN_RCVD------------------------------------ */
    SetHeader(hdr,0,0,SYN|ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "握手2发送失败..." << endl;
        return;
    }
    cout << "握手2发送成功..." << endl;

    /* ---------------------------------ESTABLISHED----------------------------------- */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "握手2发送失败..." << endl;
        return;
    }
    memcpy(&hdr,buf,sizeof(hdr));
    if (Test_Checksum(buf) && (hdr.Flags == ACK)&&(hdr.ack==0)&&(hdr.seq==1)) {
        cout << "握手3接收成功..." << endl;
    } else {
        cout << "握手3接收失败..." << endl;
        return;
    }

    cout << "服务器成功建立连接..." << endl;


    return;
}

void Server_DisConnect(SOCKET &s, sockaddr_in &addr)
{
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ESTABISHED */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "挥手1接收失败..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == (FIN|ACK))) {
        cout << "挥手1接收成功..." << endl;
    } else {
        cout << "挥手1接收失败..." << endl;
        return;
    }

    /* CLOSE_WAIT */
    SetHeader(hdr,0,0,ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "挥手2发送失败..." << endl;
        return;
    }
    cout << "挥手2发送成功..." << endl;

    /* LAST_ACK1 */
    SetHeader(hdr,0,0,FIN|ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "挥手3发送失败..." << endl;
        return;
    }
    cout << "挥手3发送成功..." << endl;

    /* LAST_ACK1 */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "挥手4接收失败..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == ACK)&&(hdr.ack==0)) {
        cout << "挥手4接收成功..." << endl;
    } else {
        cout << "挥手4接收失败..." << endl;
        return;
    }
    cout<<"服务端断开连接..."<<endl;
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


