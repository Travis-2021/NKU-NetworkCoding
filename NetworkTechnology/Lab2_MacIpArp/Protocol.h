//
// Created by lenovo on 2022/11/11.
//

#ifndef LAB2MACIPARP_PROTOCOL_H
#define LAB2MACIPARP_PROTOCOL_H
#include "pcap.h"
#pragma pack(1)
typedef struct{     // MAC帧首部
    BYTE    DesMAC[6];
    BYTE    SrcMAC[6];
    WORD    FrameType;
}FrameHeader_t;

typedef struct{     // ARP帧 （包括MAC帧头）
    FrameHeader_t   FrameHeader;
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
#pragma pack()


#endif //LAB2MACIPARP_PROTOCOL_H
