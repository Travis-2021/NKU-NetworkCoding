#include <iostream>
#include <winsock2.h>
#include "pcap.h"
#include "Protocol.h"
#include <map>
#include <string>

//#pragma comment(lib, "ws2_32.lib")

using namespace std;
#define ADDR_STR_MAX 128
#define ARP_RESPONSE 2

void Pack_ARPFrame(ARPFrame_t* ARPFrame, DWORD DstIp, DWORD SrcIp, BYTE SrcMAC[6]);
void Get_MAC(pcap_t* adhandle, BYTE TestMAC[6]);
const char *iptos(struct sockaddr *sock_addr);
void RouterConfig();
void Router(pcap_t* adhandle);
bool FindMac(DWORD ip,MAC_ADDR_t &mac);
bool MacEqual(BYTE mac1[6],BYTE mac2[6]);
void CopyMac(BYTE* Dst, BYTE* Src);

RouterTable rt;
map<DWORD,MAC_ADDR_t> ARPTable;
DWORD HostIP1;
DWORD HostIP2;
BYTE HostMAC1[6];
BYTE HostMAC2[6];

int main() {
    /* ---------------------------------0.���ػ���--------------------------------- */
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0){
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }
    /* ---------------------------------1.��ȡ�豸�б�ѡ��������--------------------------------- */
    pcap_if_t* alldevs;
    pcap_if_t* d;
    char errbuf[PCAP_ERRBUF_SIZE];
    int inum;
    int i = 0;

    if (pcap_findalldevs(&alldevs, errbuf) == -1){
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }
    for (d = alldevs; d; d = d->next){
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }
    if (i == 0){
        printf("\nNo interfaces found! Make sure Npcap is installed.\n");
        return -1;
    }
    printf("Enter the interface number (1-%d):", i);
    scanf("%d", &inum);
    if (inum < 1 || inum > i){
        printf("\nInterface number out of range.\n");
        // �ͷ��豸�б�
        pcap_freealldevs(alldevs);
        return -1;
    }

    // ��ת����ѡ������
    for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);
    // ��ȡ����IP
    const char* ip_str1 = iptos(d->addresses->addr);
    HostIP1 = inet_addr(ip_str1);
    const char* ip_str2 = iptos(d->addresses->next->addr);
    HostIP2 = inet_addr(ip_str2);
    // ��������
    pcap_t* adhandle;
    if ((adhandle = pcap_open_live(d->name, 65536, 1, 1000, errbuf)) == NULL)
    {
        fprintf(stderr, "\nUnable to open the adapter. %s is not supported by Npcap\n", d->name);
        pcap_freealldevs(alldevs);
        return -1;
    }
    pcap_freealldevs(alldevs);

    /* ---------------------------------2.���������IP��MAC��ַ--------------------------------- */
    // �������������ַ��14-F6-D8-9F-38-0A 0x14F6D89F380A
    ARPFrame_t testARP;
    BYTE TestMAC1[6] = {0x66,0x66,0x66,0x66,0x66,0x66};
    Pack_ARPFrame(&testARP, HostIP1, inet_addr("112.112.112.112"), TestMAC1);
    pcap_sendpacket(adhandle, (u_char *)&testARP, sizeof(ARPFrame_t));
    Get_MAC(adhandle, TestMAC1);
    CopyMac(HostMAC1,TestMAC1);

    BYTE TestMAC2[6] = {0x66,0x66,0x66,0x66,0x66,0x66};
    Pack_ARPFrame(&testARP, HostIP2, inet_addr("112.112.112.112"), TestMAC2);
    pcap_sendpacket(adhandle, (u_char *)&testARP, sizeof(ARPFrame_t));
    Get_MAC(adhandle, TestMAC2);
    CopyMac(HostMAC2,TestMAC2);
    cout<<"����IP��ַ��";
    PrintIP(HostIP1);
    cout<<"    ����MAC��ַ��";
    PrintMac(HostMAC1);
    cout<<"����IP��ַ��";
    PrintIP(HostIP2);
    cout<<"    ����MAC��ַ��";
    PrintMac(HostMAC2);


    cout<<"Welcome to use the SwaggyRouter..."<<endl;
    cout<<"You could use the command below to operate SwaggyRouter..."<<endl;
    PrintCommand();
    string cmd;
    while(true){
        cout<<"(enable):";
        cin>>cmd;
        if(cmd == "route-config"){
            RouterConfig();
        } else if(cmd == "route"){
            Router(adhandle);
        } else if(cmd == "exit"){
            break;
        } else if(cmd == "help") {
            PrintCommand();
        }
        else{
            cout<<"Unknown command...Please try the command provided."<<endl;
        }
    }

}

void Pack_ARPFrame(ARPFrame_t* ARPFrame, DWORD DstIp, DWORD SrcIp, BYTE SrcMAC[6])
{
    // MAC֡ͷ
    memset(ARPFrame->FrameHeader.DstMAC, 0xff, 6);
    ARPFrame->FrameHeader.SrcMAC[0] = SrcMAC[0];
    ARPFrame->FrameHeader.SrcMAC[1] = SrcMAC[1];
    ARPFrame->FrameHeader.SrcMAC[2] = SrcMAC[2];
    ARPFrame->FrameHeader.SrcMAC[3] = SrcMAC[3];
    ARPFrame->FrameHeader.SrcMAC[4] = SrcMAC[4];
    ARPFrame->FrameHeader.SrcMAC[5] = SrcMAC[5];
    ARPFrame->FrameHeader.FrameType = htons(0x0806);

    // ARP�ײ�
    ARPFrame->HardwareType = htons(0x0001);
    ARPFrame->ProtocolType = htons(0x0800);
    ARPFrame->HLen = 6;
    ARPFrame->PLen = 4;
    ARPFrame->Operation = htons(0x0001); //ARP����

    ARPFrame->SendHa[0] = SrcMAC[0];
    ARPFrame->SendHa[1] = SrcMAC[1];
    ARPFrame->SendHa[2] = SrcMAC[2];
    ARPFrame->SendHa[3] = SrcMAC[3];
    ARPFrame->SendHa[4] = SrcMAC[4];
    ARPFrame->SendHa[5] = SrcMAC[5];

    ARPFrame->SendIP = SrcIp;
    memset(ARPFrame->RecvHa, 0x00, 6);
    ARPFrame->RecvIP = DstIp;
}

void Get_MAC(pcap_t* adhandle, BYTE MAC[6])
{
    pcap_pkthdr* header;
    const u_char* pkt_data;
    while(true){
        pcap_next_ex(adhandle, &header, &pkt_data);
        // ��������
        ARPFrame_t* ARPFrame = (ARPFrame_t *)pkt_data;
        if( (ntohs(ARPFrame->FrameHeader.FrameType)==0x0806)&&
            (ntohs(ARPFrame->Operation) == ARP_RESPONSE)){
                MAC[0] = ARPFrame->SendHa[0];
                MAC[1] = ARPFrame->SendHa[1];
                MAC[2] = ARPFrame->SendHa[2];
                MAC[3] = ARPFrame->SendHa[3];
                MAC[4] = ARPFrame->SendHa[4];
                MAC[5] = ARPFrame->SendHa[5];
            break;
        }
    }
}

// Ip תΪ �ַ���
const char *iptos(struct sockaddr *sock_addr) {
    static char address[ADDR_STR_MAX] = {0};
    int gni_error = getnameinfo(sock_addr, sizeof(struct sockaddr_storage), address, ADDR_STR_MAX, nullptr, 0,
                                NI_NUMERICHOST);
    if (gni_error != 0) {
        cerr << "getnameinfo: " << gai_strerror(gni_error) << endl;
        return "ERROR!";
    }
    return address;
}

void RouterConfig(){
    string CfgCmd,DstNetwork,Mask,NextHop;
    RouterTableEntry_t e;
    while(true){
        cout<<"(route-config):";
        cin>>CfgCmd;
        if(CfgCmd=="exit"){
            break;
        }else if(CfgCmd=="showtable"){
            rt.ShowTable();
        }else if(CfgCmd=="add"){
            cin>>DstNetwork>>Mask>>NextHop;
            SetRouterTableEntry(e,inet_addr(DstNetwork.c_str()),inet_addr(Mask.c_str()),inet_addr(NextHop.c_str()));
            rt.AddEntry(e);
        }else if(CfgCmd=="delete"){
            cin>>DstNetwork>>Mask>>NextHop;
            SetRouterTableEntry(e,inet_addr(DstNetwork.c_str()),inet_addr(Mask.c_str()),inet_addr(NextHop.c_str()));
            rt.DeleteEntry(e);
        }else{
            cout<<"Unknown command...Please try the command provided."<<endl;
        }

    }
}

void Router(pcap_t* adhandle){
    // 0.�������ݰ�
    // 1.�������ݰ���ȡĿ��IP��ַ
    // 2.��·�ɱ��ҵ���һ����
    // 3.������һ����MAC��ַ
    //      3.1 ���У�����
    //      3.2 ��û�У�����ARP������ȡMAC��ַ������<ipAddr,MacAddr>����ARPTable
    // 4.�޸����ݰ�
    // 5.�������ݰ�

    pcap_pkthdr* header;
    const u_char* pkt_data;
    MAC_ADDR_t DstMac;
    ARPFrame_t arp;
    DWORD NextHop;
    int Pktnum = 0;
    while(true) {
        // 0.�������ݰ�
        pcap_next_ex(adhandle, &header, &pkt_data);
        // 1.�������ݰ���ȡĿ��IP��ַ
        EthHeader_t* eth_hdr = (EthHeader_t*)pkt_data;
        IPHeader_t* ip_hdr = (IPHeader_t*)(pkt_data+sizeof(EthHeader_t));

        if(!MacEqual(eth_hdr->DstMAC,HostMAC1)){
            continue;
        }

        // 2.��·�ɱ�
        rt.FindRoute(ip_hdr->dstaddr,NextHop);
        // 3.������һ����MAC��ַ
        if(FindMac(NextHop,DstMac)){
            // �ҵ�Ŀ��IP��MAC��ַ��do nothing.
        }else{
            // û�ҵ�MAC��ַ������ARP����
            Pack_ARPFrame(&arp,NextHop,HostIP1,HostMAC1);// TODO
            if(pcap_sendpacket(adhandle, (u_char *)&arp, sizeof(ARPFrame_t))!= 0 ){
                printf("\nError sending the packet: %s\n", pcap_geterr(adhandle));
            }else{
                printf("����ARP����ɹ���\n");
            }
            Get_MAC(adhandle,DstMac.MAC);
            ARPTable[NextHop] = DstMac;
        }
        // 4.�޸����ݰ�
        memcpy(eth_hdr->SrcMAC,HostMAC1,6);
        memcpy(eth_hdr->DstMAC,DstMac.MAC,6);

        // 5.�������ݰ�
        if(memcmp((const void*)((EthHeader_t*)pkt_data)->SrcMAC,(const void*)HostMAC1,6)== 0 ) {
            if(pcap_sendpacket(adhandle, (u_char *)pkt_data, header->len)!= 0 ){
                printf("\nError sending the packet: %s\n", pcap_geterr(adhandle));
            }else{
                cout<<"��"<<++Pktnum<<"�����ݰ���  ԴIP��ַ��";
                PrintIP(ip_hdr->srcaddr);
                cout<<"\tĿ��IP��ַ��";
                PrintIP(ip_hdr->dstaddr);
                cout<<"\t��һ��IP��ַ��";
                PrintIP(NextHop);
                cout<<"\tĿ��MAC��ַ��";
                PrintMac(DstMac.MAC);
                cout<<endl;
            }
        }


    }

}

bool FindMac(DWORD ip,MAC_ADDR_t &mac){
    auto iter = ARPTable.find(ip);
    if(iter!=ARPTable.end()){
        mac = iter->second;
        return true;
    }else{
        return false;
    }
}

bool MacEqual(BYTE mac1[6],BYTE mac2[6]){
    if((mac1[0]==mac2[0])&&(mac1[1]==mac2[1])
    &&(mac1[2]==mac2[2])&&(mac1[3]==mac2[3])
    &&(mac1[4]==mac2[4])&&(mac1[5]==mac2[5]))
        return true;
    else
        return false;
}

void CopyMac(BYTE* Dst, BYTE* Src){
    Dst[0] = Src[0];
    Dst[1] = Src[1];
    Dst[2] = Src[2];
    Dst[3] = Src[3];
    Dst[4] = Src[4];
    Dst[5] = Src[5];
}


