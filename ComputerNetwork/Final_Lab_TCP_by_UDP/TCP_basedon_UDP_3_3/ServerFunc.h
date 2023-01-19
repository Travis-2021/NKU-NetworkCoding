#ifndef TCP_BASEDON_UDP_3_2_SERVERFUNC_H
#define TCP_BASEDON_UDP_3_2_SERVERFUNC_H
#include<iostream>
#include<winsock2.h>
#include"Support.h"
using namespace std;

void Server_Connect(SOCKET &s, sockaddr_in &addr) {
    HEADER hdr;
    char buf[sizeof(HEADER)];
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



#endif //TCP_BASEDON_UDP_3_2_SERVERFUNC_H
