#ifndef LAB5_ROUTERPROGRAM_PROTOCOL_H
#define LAB5_ROUTERPROGRAM_PROTOCOL_H

#include <winsock2.h>
#include "pcap.h"
#include <vector>
#include <algorithm>


using namespace std;

#pragma pack(1)
// MAC��ַ
typedef struct{
    BYTE MAC[6];
}MAC_ADDR_t;

// Eth֡�ײ�
typedef struct{
    BYTE    DstMAC[6];
    BYTE    SrcMAC[6];
    WORD    FrameType;
}EthHeader_t;

// ARP֡ ������MAC֡ͷ��
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
    unsigned char ihl:4;     //�ײ�����
	unsigned char version:4; //�汾
    unsigned char tos;       //��������
    unsigned short tot_len;  //�ܳ���
    unsigned short id;       //��־
    unsigned short frag_off; //��Ƭƫ��
    unsigned char ttl;       //����ʱ��
    unsigned char protocol;  //Э��
    unsigned short chk_sum;  //�����
    DWORD srcaddr;  //ԴIP��ַ
    DWORD dstaddr;  //Ŀ��IP��ַ
}IPHeader_t;


// ·�ɱ����
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
    vector<RouterTableEntry_t> RouterArray;     // ·�ɱ�
public:
    RouterTable(){
        InitRouterTable();
    }
    void InitRouterTable(){
        // ���Ĭ��·��,������main.cpp��д
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
                // �ж��Ƿ���Ĭ��·�ɣ�ֱ��Ͷ��
                if( (i->DstNetwork == inet_addr("0.0.0.0"))||
                    (i->DstNetwork == inet_addr("206.1.1.0"))||
                    (i->DstNetwork == inet_addr("206.1.2.0"))
                        ){
                    cout<<"Ĭ��·�ɲ���ɾ��"<<endl;
                }else{
                    RouterArray.erase(i);
                    cout<<"ɾ���ɹ���"<<endl;
                }
                return;
            }
        }
        cout<<"δ�ҵ�ƥ������������Ƿ���ȷ"<<endl;
        return;
    }
    void ShowTable(){
        cout<<"Ŀ������      ��������         ��һ��"<<endl;
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
            // �ж��Ƿ�����Ŀ������
            if((Dstip&iter->Mask)==iter->DstNetwork){
                NextHop = iter->NextHop;
                find = true;
                break;
            }
            iter++;
        }
        // û�ҵ���ʹ��Ĭ��·��
        if(!find){
            NextHop = RouterArray.begin()->NextHop;
        }
    }

};




#endif //LAB5_ROUTERPROGRAM_PROTOCOL_H
