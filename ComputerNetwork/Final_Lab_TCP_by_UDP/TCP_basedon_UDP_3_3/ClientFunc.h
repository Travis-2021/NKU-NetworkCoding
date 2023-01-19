#ifndef TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
#define TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
#include<iostream>
#include<winsock2.h>
#include"Support.h"
#include"FileUtil.h"
using namespace std;



/* 记录窗口变化 */
u_short current_seq = 0;
u_short current_ack = current_seq;
int base = current_seq;
int nextseqnum = base;

/* 用于描述数据包与文件 */
int sum=0;
int ReadSize = 0;
u_long FileSize=0;
int num = 0;

/* 窗口对应缓冲区 */
Pkt pkt[WinSize];
char pktbuf[DataMaxSize + sizeof(HEADER)];
int currentpkt = 0;

/* 线程信息 */
bool SendFile = false;
bool OnTimer = false;
clock_t start=clock();


void printWin(){
//    u_long WinTop = base+WinSize<num? base+WinSize:num;
//    cout<<"窗口[base,nextseqnum,base+N]：["<<base<<","<<nextseqnum<<","<<WinTop<<"]"<<endl;
    u_long WinTop = base+cwnd<num? base+cwnd:num;
    cout<<"窗口[base,nextseqnum,base+N]：["<<base<<","<<nextseqnum<<","<<WinTop<<"]"<<endl;
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
        cout << "握手1发送失败..." << endl;
        return;
    }
    // 启动计时器
//    thread Timer1(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer1.detach();
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
//    thread Timer2(TimerThread, ref(s),ref(addr),buf, sizeof(hdr));
//    Timer2.detach();
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

void ClientSendThread(SOCKET& s, sockaddr_in& addr){
    /* ------------------------------选择文件-------------------------------------------*/
    int fileNum = 1;
    FileList fileList = Dir::entry_static("../test/");
    for (auto &file: fileList) {
        cout << fileNum++ << ". " << file->fileName() << endl;
    }
    char Filepath[] = "../test/";
    char FileName[20];
    cout<<"请输入文件名称..."<<endl;
    cin>>FileName;
    strcat(Filepath,FileName);

    /* ------------------------------打开文件-------------------------------------------*/
    ifstream inFile(Filepath,ios::in|ios::binary);
    if(!inFile.is_open()){
        cout<<"打开文件失败"<<endl;
        inFile.close();
    }
    /* ------------------------------发送描述信息------------------------------------------*/

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
        // 窗口内数据未发送完
        if(nextseqnum<base+WinSize){
            // 判断是否为最后一个包，设置包大小
            ReadSize = (FileSize-sum)<DataMaxSize? (FileSize-sum):DataMaxSize;
            SetHeader(hdr,current_seq,0,0,ReadSize);
            if(i==num){
                hdr.Flags = END;
            }

            // 向当前窗口项写数据，并发送
            pkt[currentpkt].hdr = hdr;
            inFile.read(pkt[currentpkt].data,ReadSize);
            memcpy(pktbuf,&pkt[currentpkt],sizeof(pkt[i]));
            updateCurrentpkt();
            Add_Checksum(pktbuf);
            sendto(s,pktbuf,sizeof(pktbuf),0,(sockaddr*)&addr,sizeof(addr));
            cout<<"发送数据包"<<hdr.seq<<"，等待确认"<<endl;
            current_seq++;

            //如果是窗口头，则开启定时器
            if(base == nextseqnum){
                start_Timer();
                cout<<"启动定时器..."<<endl;
            }
            nextseqnum++;
            printWin();

            i++;
            // 最后一包发送完毕
            if(i>num) {
                break;
            }
        }

    }
    Fileend = clock();
    cout<<"总传输时间："<<double(Fileend-Filestart)/1000<<"s"<<endl;
    cout<<"平均吞吐率："<<double(FileSize)/(Fileend-Filestart)<<"Kb/s"<<endl;

}

void ClientRecvThread(SOCKET& s, sockaddr_in& addr){
    HEADER hdr;
    char hdrbuf[sizeof(HEADER)];
    int addrlen = sizeof(addr);


    while(true){
        recvfrom(s, hdrbuf, sizeof(hdrbuf), 0, (sockaddr *) &addr, &addrlen);
        RECIVE_ACK = true;
        memcpy(&hdr,hdrbuf,sizeof(hdrbuf));

        // 确认是ACK
        if((hdr.Flags==ACK)&&(hdr.ack==current_ack)&&Test_Checksum(hdrbuf)){
            sum += ReadSize;
            cout<<"已成功收到确认号"<<hdr.ack<<"目前已发送"<<sum<<"Bytes..."<<endl;
            if(hdr.ack==num-1)
                break;
            current_ack++;

            // 更新并显示base
            base = hdr.ack+1;
            printWin();

            // 拥塞控制窗口更新
            cwnd = cwnd>=ssthresh? cwnd+1:cwnd*2;
            PrintWndSst(cwnd,ssthresh);

            // 窗口全部发送
            if (base == current_seq) {
                //停止定时器
                stop_Timer();
            } else {
                //开启定时器
                start_Timer();
            }


        }

    }
    SendFile = false;
    cout<<"客户端接收线程停止..."<<endl;
}



void ReSend(SOCKET& s, sockaddr_in& addr){
    for(int i=base;i<nextseqnum;i++){
        int pktindex = i%WinSize;
        memcpy(pktbuf,&pkt[pktindex],sizeof(pkt[pktindex]));
        sendto(s,pktbuf,pkt[pktindex].hdr.Size+sizeof(HEADER),0,(sockaddr*)&addr,sizeof(addr));
        cout<<"重发数据包"<<i<<endl;
    }
}

void ClientTimerThread(SOCKET& s, sockaddr_in& addr){

    while(SendFile){
        if(OnTimer&&((GetTickCount() - start) > RTO)){
            // 超时，可能发生拥塞，更新拥塞控制窗口
            ssthresh = cwnd/2;
            cwnd = 1;
            PrintWndSst(cwnd,ssthresh);

            cout<<"超时重传..."<<endl;
            start_Timer();
            ReSend(s,addr);
        }
    }

}





#endif //TCP_BASEDON_UDP_3_2_CLIENTFUNC_H
