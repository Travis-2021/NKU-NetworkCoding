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
    addr->sin_port = htons(port);  // �������˿�
    if (ip_str == NULL) {
        addr->sin_addr.S_un.S_addr = INADDR_ANY;
    } else {
        addr->sin_addr.S_un.S_addr = inet_addr(ip_str);
    }
}

void Add_Checksum(char* buf) {
    HEADER* hdr = (HEADER*)buf;
    int TotalSize = sizeof(HEADER) + hdr->Size;     // Header+Data��С
    int TotalBlock = (TotalSize%2==0)? TotalSize/2:TotalSize/2+1; // ��Ҫ����������������ȡ��
    WORD* data = (WORD*)buf; // 16λ����
    DWORD temp = 0;
    for(int i=0;i<TotalBlock;i++){
        temp += data[i];
        if(temp > 0xFFFF){  // ������λ�������λ���������
            temp += temp>>0xFFFF;
            temp &= 0xFFFF;
        }
    }
    hdr->Checksum = (~temp)&0xFFFF; // ȡ��16λ��ȡ��
}

bool Test_Checksum(char* buf){
    HEADER *hdr = (HEADER *) buf;
    int TotalSize = sizeof(HEADER) + hdr->Size;     // Header+Data��С
    int TotalBlock = (TotalSize % 2 == 0) ? TotalSize / 2 : TotalSize / 2 + 1; // ��Ҫ����������������ȡ��
    WORD *data = (WORD *) buf; // 16λ����
    DWORD temp = 0;
    for (int i = 0; i < TotalBlock; i++) {
        temp += data[i];
        if (temp > 0xFFFF) {  // ������λ�������λ���������
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
        cout << "����1����ʧ��..." << endl;
        return;
    }
    // ������ʱ��
    thread Timer1(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
    Timer1.detach();
    //start = clock();
    cout << "����1���ͳɹ�..." << endl;

    /* -----------------------------------SYN_SENT----------------------------------- */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"����2����ʧ��..."<<endl;
        return;
    }

    //end = clock();
    //cout<<"RTT:"<<(end-start)<<endl; //RTT == 3;

    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(SYN|ACK))&&(hdr.ack==0)){
        cout<<"����2���ճɹ�..."<<endl;
    }else{
        cout<<"����2����ʧ��..."<<endl;
        return;
    }

    /* ---------------------------------ESTABLISHED----------------------------------- */
    // ���͵�3������
    SetHeader(hdr,1,0,ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����3����ʧ��..." << endl;
        return;
    }
    // ������ʱ��
    thread Timer2(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
    Timer2.detach();
    cout << "����3���ͳɹ�..." << endl;
    cout << "�ͻ��˳ɹ���������..." << endl;

    return;
}

void Client_DisConnect(SOCKET &s, sockaddr_in &addr)
{
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ESTABLISHED״̬ */
    SetHeader(hdr,0,0,FIN|ACK,0);
    memcpy(buf, &hdr, sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����1ʧ��..." << endl;
        return;
    }
    cout<<"����1���ͳɹ�..."<<endl;

//    // ������ʱ��
//    thread Timer3(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer3.detach();

    /* FIN_WAIT1 */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"����2����ʧ��..."<<endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(ACK))&&(hdr.ack==0)){
        cout<<"����2���ճɹ�..."<<endl;
    }else{
        cout<<"����2����ʧ��..."<<endl;
        return;
    }

    /* FIN_WAIT2 */
    if (recvfrom(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, &addrlen)== -1) {
        cout<<"����3����ʧ��..."<<endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));
    if(Test_Checksum(buf)&&(hdr.Flags==(FIN|ACK))){
        cout<<"����3���ճɹ�..."<<endl;
    }else{
        cout<<"����3����ʧ��..."<<endl;
        return;
    }

    /* TIME_WAIT״̬ */
    SetHeader(hdr,1,0,ACK,0);
    memcpy(buf, &hdr, sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����4ʧ��..." << endl;
        return;
    }
//    // ������ʱ��
//    thread Timer4(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer4.detach();

    cout<<"����4���ͳɹ�..."<<endl;
    cout<<"�ͻ��˶Ͽ�����..."<<endl;
}

void Server_Connect(SOCKET &s, sockaddr_in &addr) {
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ------------------------------------LISTEN---------------------------------- */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "����1����ʧ��..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == SYN)&&(hdr.seq==0)) {
        cout << "����1���ճɹ�..." << endl;
    } else {
        cout << "����1����ʧ��..." << endl;
        return;
    }

    /* ----------------------------------SYN_RCVD------------------------------------ */
    SetHeader(hdr,0,0,SYN|ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����2����ʧ��..." << endl;
        return;
    }
    cout << "����2���ͳɹ�..." << endl;

    /* ---------------------------------ESTABLISHED----------------------------------- */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "����2����ʧ��..." << endl;
        return;
    }
    memcpy(&hdr,buf,sizeof(hdr));
    if (Test_Checksum(buf) && (hdr.Flags == ACK)&&(hdr.ack==0)&&(hdr.seq==1)) {
        cout << "����3���ճɹ�..." << endl;
    } else {
        cout << "����3����ʧ��..." << endl;
        return;
    }

    cout << "�������ɹ���������..." << endl;


    return;
}

void Server_DisConnect(SOCKET &s, sockaddr_in &addr)
{
    HEADER hdr;
    char buf[sizeof(hdr)];
    int addrlen = sizeof(addr);

    /* ESTABISHED */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "����1����ʧ��..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == (FIN|ACK))) {
        cout << "����1���ճɹ�..." << endl;
    } else {
        cout << "����1����ʧ��..." << endl;
        return;
    }

    /* CLOSE_WAIT */
    SetHeader(hdr,0,0,ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����2����ʧ��..." << endl;
        return;
    }
    cout << "����2���ͳɹ�..." << endl;

    /* LAST_ACK1 */
    SetHeader(hdr,0,0,FIN|ACK,0);
    memcpy(buf,&hdr,sizeof(hdr));
    Add_Checksum(buf);

    if (sendto(s, buf, sizeof(hdr), 0, (sockaddr *) &addr, sizeof(addr)) == -1) {
        cout << "����3����ʧ��..." << endl;
        return;
    }
    cout << "����3���ͳɹ�..." << endl;

    /* LAST_ACK1 */
    if (recvfrom(s, buf, sizeof(buf), 0, (sockaddr *) &addr, &addrlen) == -1) {
        cout << "����4����ʧ��..." << endl;
        return;
    }
    memcpy(&hdr, buf, sizeof(hdr));

    if (Test_Checksum(buf) && (hdr.Flags == ACK)&&(hdr.ack==0)) {
        cout << "����4���ճɹ�..." << endl;
    } else {
        cout << "����4����ʧ��..." << endl;
        return;
    }
    cout<<"����˶Ͽ�����..."<<endl;
}

void TimerThread(SOCKET& s, sockaddr_in& addr, char *buf, int len){
    clock_t start=clock();
    while(!RECIVE_ACK){
        clock_t end=clock();
        if((end-start)>RTO) {
            /* ���·��� */
            sendto(s,buf,len,0,(sockaddr*)&addr,sizeof(addr));
            cout<<"��������ʱ�ش�..."<<endl;
            start=clock();
        }
    }
}


