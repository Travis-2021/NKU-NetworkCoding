#include <iostream>
#include "pcap.h"
#include "Protocol.h"
#include <winsock2.h>
using namespace std;
#define ADDR_STR_MAX 128
#define ARP_RESPONSE 2

void Capture_ARP(pcap_t* adhandle, DWORD DstIp);
void Print_IP_MAC(DWORD ip, BYTE* mac);
void Pack_ARPFrame(ARPFrame_t* ARPFrame, DWORD DstIp, DWORD SrcIp, BYTE SrcMAC[6]);
const char *iptos(struct sockaddr *sock_addr);
void Get_MAC(pcap_t* adhandle, BYTE TestMAC[6]);

int main() {
    /*
     * 0. 加载环境
     * 1. 获取设备列表，选择适配器
     * 2. 获得适配器IP、MAC地址
     * 3. 组装并发送ARP请求报文
     * 4. 解析ARP响应报文
     * */


    /* 0.加载环境 */
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
    {
        cout << "WSAStartup Error:" << WSAGetLastError() << endl;
        return 0;
    }

    /* 1.获取设备列表，选择适配器 */
    pcap_if_t* alldevs;
    pcap_if_t* d;
    char errbuf[PCAP_ERRBUF_SIZE];

    int inum;
    int i = 0;

    // 获取、打印设备列表
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }
    for (d = alldevs; d; d = d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }
    if (i == 0)
    {
        printf("\nNo interfaces found! Make sure Npcap is installed.\n");
        return -1;
    }

    printf("Enter the interface number (1-%d):", i);
    scanf("%d", &inum);

    if (inum < 1 || inum > i)
    {
        printf("\nInterface number out of range.\n");
        // 释放设备列表
        pcap_freealldevs(alldevs);
        return -1;
    }

    // 跳转到所选适配器
    for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

    // 获取本机IP
    const char* ip_str = iptos(d->addresses->addr);
    DWORD SrcIp = inet_addr(ip_str);

    // 打开适配器
    pcap_t* adhandle;
    if ((adhandle = pcap_open_live(d->name, 65536, 1, 1000, errbuf)) == NULL)
    {
        fprintf(stderr, "\nUnable to open the adapter. %s is not supported by Npcap\n", d->name);

        pcap_freealldevs(alldevs);
        return -1;
    }
    pcap_freealldevs(alldevs);


    /* 2.获取适配器MAC */

    // 本机网卡物理地址：14-F6-D8-9F-38-0A 0x14F6D89F380A
    ARPFrame_t testARP;
    BYTE TestMAC[6] = {0x66,0x66,0x66,0x66,0x66,0x66};
    Pack_ARPFrame(&testARP, SrcIp, inet_addr("112.112.112.112"), TestMAC);

    pcap_sendpacket(adhandle, (u_char *)&testARP, sizeof(ARPFrame_t));
    printf("发送测试ARP请求报文成功！\n");

    Get_MAC(adhandle, TestMAC);

    Print_IP_MAC(SrcIp, TestMAC);


    //BYTE SrcMAC[6] = {0x14,0xf6,0xd8,0x9f,0x38,0x0a};
    BYTE *SrcMAC = TestMAC;

    /* 3.组装、发送ARP帧 */

    char DstIp_str[ADDR_STR_MAX] = {0};

    ARPFrame_t ARPFrame;

    while(true)
    {
        printf("请输入目的IP地址...输入exit退出\n");
        scanf("%s", &DstIp_str);
        if(strcmp(DstIp_str, "exit") == 0)
            break;
        else{
            DWORD DstIp = inet_addr(DstIp_str);
            Pack_ARPFrame(&ARPFrame, DstIp, SrcIp, SrcMAC);

            if(pcap_sendpacket(adhandle, (u_char *)&ARPFrame, sizeof(ARPFrame_t))!= 0 ){
                printf("\nError sending the packet: %s\n", pcap_geterr(adhandle));
            }else{
                printf("发送成功！\n");
            }
            /* 4.解析ARP包 */
            Capture_ARP(adhandle,DstIp);
        }
    }

    WSACleanup();
    return 0;
}

// Ip 转为 字符串
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

void Pack_ARPFrame(ARPFrame_t* ARPFrame, DWORD DstIp, DWORD SrcIp, BYTE SrcMAC[6])
{
    // MAC帧头
    memset(ARPFrame->FrameHeader.DesMAC, 0xff, 6);
    ARPFrame->FrameHeader.SrcMAC[0] = SrcMAC[0];
    ARPFrame->FrameHeader.SrcMAC[1] = SrcMAC[1];
    ARPFrame->FrameHeader.SrcMAC[2] = SrcMAC[2];
    ARPFrame->FrameHeader.SrcMAC[3] = SrcMAC[3];
    ARPFrame->FrameHeader.SrcMAC[4] = SrcMAC[4];
    ARPFrame->FrameHeader.SrcMAC[5] = SrcMAC[5];
    ARPFrame->FrameHeader.FrameType = htons(0x0806);

    // ARP首部
    ARPFrame->HardwareType = htons(0x0001);
    ARPFrame->ProtocolType = htons(0x0800);
    ARPFrame->HLen = 6;
    ARPFrame->PLen = 4;
    ARPFrame->Operation = htons(0x0001); //ARP请求

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

void Capture_ARP(pcap_t* adhandle, DWORD DstIp)
{

    pcap_pkthdr* header;
    const u_char* pkt_data;

    while(true)
    {
        pcap_next_ex(adhandle, &header, &pkt_data);

        // 解析报文
        ARPFrame_t* ARPFrame = (ARPFrame_t *)pkt_data;
        if( (ntohs(ARPFrame->FrameHeader.FrameType)==0x0806)&&
            (ntohs(ARPFrame->Operation) == ARP_RESPONSE) &&
            (ARPFrame->SendIP == DstIp)){
            Print_IP_MAC(ARPFrame->SendIP, ARPFrame->SendHa);
            break;
        }
    }
}

void Print_IP_MAC(DWORD ip, BYTE* mac)
{
    printf("IP地址：");
    printf(" %lu", (ip>>0)&0xff);
    printf(".%lu", (ip>>8)&0xff);
    printf(".%lu", (ip>>16)&0xff);
    printf(".%lu", (ip>>24)&0xff);

    printf("      MAC地址：");
    printf(" %02x", mac[0]);
    printf(":%02x", mac[1]);
    printf(":%02x", mac[2]);
    printf(":%02x", mac[3]);
    printf(":%02x", mac[4]);
    printf(":%02x\n", mac[5]);
}

void Get_MAC(pcap_t* adhandle, BYTE TestMAC[6])
{
    pcap_pkthdr* header;
    const u_char* pkt_data;

    while(true)
    {
        pcap_next_ex(adhandle, &header, &pkt_data);

        // 解析报文
        ARPFrame_t* ARPFrame = (ARPFrame_t *)pkt_data;
        if( (ntohs(ARPFrame->FrameHeader.FrameType)==0x0806)&&
            (ntohs(ARPFrame->Operation) == ARP_RESPONSE)){
            {
                TestMAC[0] = ARPFrame->SendHa[0];
                TestMAC[1] = ARPFrame->SendHa[1];
                TestMAC[2] = ARPFrame->SendHa[2];
                TestMAC[3] = ARPFrame->SendHa[3];
                TestMAC[4] = ARPFrame->SendHa[4];
                TestMAC[5] = ARPFrame->SendHa[5];
            }
            break;
        }
    }
}
