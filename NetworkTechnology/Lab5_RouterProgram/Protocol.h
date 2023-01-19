#ifndef LAB5_ROUTERPROGRAM_PROTOCOL_H
#define LAB5_ROUTERPROGRAM_PROTOCOL_H

#include <winsock2.h>
#include "pcap.h"
#include <vector>
#include <algorithm>


using namespace std;

#pragma pack(1)
// MAC地址
typedef struct{
    BYTE MAC[6];
}MAC_ADDR_t;

// Eth帧首部
typedef struct{
    BYTE    DstMAC[6];
    BYTE    SrcMAC[6];
    WORD    FrameType;
}EthHeader_t;

// ARP帧 （包括MAC帧头）
typedef struct{
    EthHeader_t   FrameHeader;
    WORD            HardwareType;
    WORD            ProtocolType;
    BYTE            HLen;
    BYTE            PLen;
    WORD            Operation;
    BYTE            SendHa[6];
    DWORD           SendIP;
    BYTE            RecvHa[6];
    DWORD           RecvIP;
}ARPFrame_t;

typedef struct{
    unsigned char ihl:4;     //首部长度
	unsigned char version:4; //版本
    unsigned char tos;       //服务类型
    unsigned short tot_len;  //总长度
    unsigned short id;       //标志
    unsigned short frag_off; //分片偏移
    unsigned char ttl;       //生存时间
    unsigned char protocol;  //协议
    unsigned short chk_sum;  //检验和
    DWORD srcaddr;  //源IP地址
    DWORD dstaddr;  //目的IP地址
}IPHeader_t;


// 路由表表项
typedef struct{
    DWORD DstNetwork;
    DWORD Mask;
    DWORD NextHop;
}RouterTableEntry_t;

#pragma pack()

void PrintIP(DWORD ip){
    printf("%lu", (ip>>0)&0xff);
    printf(".%lu", (ip>>8)&0xff);
    printf(".%lu", (ip>>16)&0xff);
    printf(".%lu", (ip>>24)&0xff);
}
void PrintMac(BYTE* mac){
    printf("%02x", mac[0]);
    printf(":%02x", mac[1]);
    printf(":%02x", mac[2]);
    printf(":%02x", mac[3]);
    printf(":%02x", mac[4]);
    printf(":%02x\n", mac[5]);
}
void PrintCommand(){
    cout<<"route-config:\tSet up the RouterTable."<<endl;
    cout<<"route:\t\tRoute packages."<<endl;
    cout<<"exit:\t\tExit the Program."<<endl;
    cout<<"help:\t\tGet the command provided."<<endl;
}
void SetMac(BYTE* DstMac, BYTE* SrcMac){
    DstMac[0] = SrcMac[0];
    DstMac[1] = SrcMac[1];
    DstMac[2] = SrcMac[2];
    DstMac[3] = SrcMac[3];
    DstMac[4] = SrcMac[4];
    DstMac[5] = SrcMac[5];
}

void SetRouterTableEntry(RouterTableEntry_t &e,DWORD DstNetwork,DWORD Mask,DWORD NextHop){
    e.DstNetwork=DstNetwork;
    e.Mask=Mask;
    e.NextHop=NextHop;
}

bool CmpEntry(RouterTableEntry_t a, RouterTableEntry_t b){
    return a.Mask>b.Mask;
}


class RouterTable{
    vector<RouterTableEntry_t> RouterArray;     // 路由表
public:
    RouterTable(){
        InitRouterTable();
    }
    void InitRouterTable(){
        // 添加默认路由,或者在main.cpp里写
        RouterTableEntry_t DefaultRouter = {0,0,inet_addr("206.1.2.2")};
        AddEntry(DefaultRouter);
        RouterTableEntry_t e1 = {inet_addr("206.1.1.0"),inet_addr("255.255.255.0"),inet_addr("206.1.1.2")};
        AddEntry(e1);
        RouterTableEntry_t e2 = {inet_addr("206.1.2.0"),inet_addr("255.255.255.0"),inet_addr("206.1.2.2")};
        AddEntry(e2);
    }

    void AddEntry(RouterTableEntry_t e){
        RouterArray.push_back(e);
        sort(RouterArray.begin()+1,RouterArray.end(), CmpEntry);
    }
    void DeleteEntry(RouterTableEntry_t e){
        for(auto i=RouterArray.begin();i<RouterArray.end();i++){
            if((i->DstNetwork==e.DstNetwork)&&(i->Mask==e.Mask)&&(i->NextHop==e.NextHop)){
                // 判断是否是默认路由，直接投递
                if( (i->DstNetwork == inet_addr("0.0.0.0"))||
                    (i->DstNetwork == inet_addr("206.1.1.0"))||
                    (i->DstNetwork == inet_addr("206.1.2.0"))
                        ){
                    cout<<"默认路由不可删除"<<endl;
                }else{
                    RouterArray.erase(i);
                    cout<<"删除成功！"<<endl;
                }
                return;
            }
        }
        cout<<"未找到匹配项，请检查输入是否正确"<<endl;
        return;
    }
    void ShowTable(){
        cout<<"目的网络      子网掩码         下一跳"<<endl;
        for(auto i=RouterArray.begin();i<RouterArray.end();i++){
            PrintIP(i->DstNetwork);
            cout<<"   ";
            PrintIP(i->Mask);
            cout<<"   ";
            PrintIP(i->NextHop);
            cout<<endl;
        }
    }

    void FindRoute(DWORD Dstip,DWORD &NextHop){
        bool find = false;
        auto iter = RouterArray.begin()+1;
        while(iter!=RouterArray.end()){
            // 判断是否属于目的网络
            if((Dstip&iter->Mask)==iter->DstNetwork){
                NextHop = iter->NextHop;
                find = true;
                break;
            }
            iter++;
        }
        // 没找到，使用默认路由
        if(!find){
            NextHop = RouterArray.begin()->NextHop;
        }
    }

};




#endif //LAB5_ROUTERPROGRAM_PROTOCOL_H
