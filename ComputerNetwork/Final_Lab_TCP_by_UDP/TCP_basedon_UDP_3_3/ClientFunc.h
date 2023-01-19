#ifndef TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
#define TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
#include<iostream>
#include<winsock2.h>
#include"Support.h"
#include"FileUtil.h"
using namespace std;



/* ��¼���ڱ仯 */
u_short current_seq = 0;
u_short current_ack = current_seq;
int base = current_seq;
int nextseqnum = base;

/* �����������ݰ����ļ� */
int sum=0;
int ReadSize = 0;
u_long FileSize=0;
int num = 0;

/* ���ڶ�Ӧ������ */
Pkt pkt[WinSize];
char pktbuf[DataMaxSize + sizeof(HEADER)];
int currentpkt = 0;

/* �߳���Ϣ */
bool SendFile = false;
bool OnTimer = false;
clock_t start=clock();


void printWin(){
//    u_long WinTop = base+WinSize<num? base+WinSize:num;
//    cout<<"����[base,nextseqnum,base+N]��["<<base<<","<<nextseqnum<<","<<WinTop<<"]"<<endl;
    u_long WinTop = base+cwnd<num? base+cwnd:num;
    cout<<"����[base,nextseqnum,base+N]��["<<base<<","<<nextseqnum<<","<<WinTop<<"]"<<endl;
}

void updateCurrentpkt(){
    currentpkt++;
    currentpkt = currentpkt%WinSize;
}

void start_Timer(){
    OnTimer = true;
    start =GetTickCount();
}

void stop_Timer() {
    OnTimer = false;
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
//    thread Timer1(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer1.detach();
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
//    thread Timer2(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer2.detach();
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

void ClientSendThread(SOCKET& s, sockaddr_in& addr){
    /* ------------------------------ѡ���ļ�-------------------------------------------*/
    int fileNum = 1;
    FileList fileList = Dir::entry_static("../test/");
    for (auto &file: fileList) {
        cout << fileNum++ << ". " << file->fileName() << endl;
    }
    char Filepath[] = "../test/";
    char FileName[20];
    cout<<"�������ļ�����..."<<endl;
    cin>>FileName;
    strcat(Filepath,FileName);

    /* ------------------------------���ļ�-------------------------------------------*/
    ifstream inFile(Filepath,ios::in|ios::binary);
    if(!inFile.is_open()){
        cout<<"���ļ�ʧ��"<<endl;
        inFile.close();
    }
    /* ------------------------------����������Ϣ------------------------------------------*/

    FileSize =  File::fileSize(Filepath);
    DESC desc;
    char descbuf[sizeof(desc)];

    SetDESC(desc,FileSize,FileSize/DataMaxSize+1,WinSize,FileName);
    PrintDESC(desc);
    memcpy(descbuf,&desc,sizeof(desc));
    sendto(s, descbuf, sizeof(descbuf), 0, (sockaddr *) &addr, sizeof(addr));
    /* -------------------------------------------------------------------------*/

    int i=1;
    num = desc.PackNum;
    HEADER hdr;

    clock_t Filestart,Fileend;
    Filestart = clock();
    while(true){
        // ����������δ������
        if(nextseqnum<base+WinSize){
            // �ж��Ƿ�Ϊ���һ���������ð���С
            ReadSize = (FileSize-sum)<DataMaxSize? (FileSize-sum):DataMaxSize;
            SetHeader(hdr,current_seq,0,0,ReadSize);
            if(i==num){
                hdr.Flags = END;
            }

            // ��ǰ������д���ݣ�������
            pkt[currentpkt].hdr = hdr;
            inFile.read(pkt[currentpkt].data,ReadSize);
            memcpy(pktbuf,&pkt[currentpkt],sizeof(pkt[i]));
            updateCurrentpkt();
            Add_Checksum(pktbuf);
            sendto(s,pktbuf,sizeof(pktbuf),0,(sockaddr*)&addr,sizeof(addr));
            cout<<"�������ݰ�"<<hdr.seq<<"���ȴ�ȷ��"<<endl;
            current_seq++;

            //����Ǵ���ͷ��������ʱ��
            if(base == nextseqnum){
                start_Timer();
                cout<<"������ʱ��..."<<endl;
            }
            nextseqnum++;
            printWin();

            i++;
            // ���һ���������
            if(i>num) {
                break;
            }
        }

    }
    Fileend = clock();
    cout<<"�ܴ���ʱ�䣺"<<double(Fileend-Filestart)/1000<<"s"<<endl;
    cout<<"ƽ�������ʣ�"<<double(FileSize)/(Fileend-Filestart)<<"Kb/s"<<endl;

}

void ClientRecvThread(SOCKET& s, sockaddr_in& addr){
    HEADER hdr;
    char hdrbuf[sizeof(HEADER)];
    int addrlen = sizeof(addr);


    while(true){
        recvfrom(s, hdrbuf, sizeof(hdrbuf), 0, (sockaddr *) &addr, &addrlen);
        RECIVE_ACK = true;
        memcpy(&hdr,hdrbuf,sizeof(hdrbuf));

        // ȷ����ACK
        if((hdr.Flags==ACK)&&(hdr.ack==current_ack)&&Test_Checksum(hdrbuf)){
            sum += ReadSize;
            cout<<"�ѳɹ��յ�ȷ�Ϻ�"<<hdr.ack<<"Ŀǰ�ѷ���"<<sum<<"Bytes..."<<endl;
            if(hdr.ack==num-1)
                break;
            current_ack++;

            // ���²���ʾbase
            base = hdr.ack+1;
            printWin();

            // ӵ�����ƴ��ڸ���
            cwnd = cwnd>=ssthresh? cwnd+1:cwnd*2;
            PrintWndSst(cwnd,ssthresh);

            // ����ȫ������
            if (base == current_seq) {
                //ֹͣ��ʱ��
                stop_Timer();
            } else {
                //������ʱ��
                start_Timer();
            }


        }

    }
    SendFile = false;
    cout<<"�ͻ��˽����߳�ֹͣ..."<<endl;
}



void ReSend(SOCKET& s, sockaddr_in& addr){
    for(int i=base;i<nextseqnum;i++){
        int pktindex = i%WinSize;
        memcpy(pktbuf,&pkt[pktindex],sizeof(pkt[pktindex]));
        sendto(s,pktbuf,pkt[pktindex].hdr.Size+sizeof(HEADER),0,(sockaddr*)&addr,sizeof(addr));
        cout<<"�ط����ݰ�"<<i<<endl;
    }
}

void ClientTimerThread(SOCKET& s, sockaddr_in& addr){

    while(SendFile){
        if(OnTimer&&((GetTickCount() - start) > RTO)){
            // ��ʱ�����ܷ���ӵ��������ӵ�����ƴ���
            ssthresh = cwnd/2;
            cwnd = 1;
            PrintWndSst(cwnd,ssthresh);

            cout<<"��ʱ�ش�..."<<endl;
            start_Timer();
            ReSend(s,addr);
        }
    }

}





#endif //TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
