#include <iostream>
#include<winsock2.h>
using namespace std;

struct HEADER { // 10 Bytes
    u_short Checksum;
    u_short seq;
    u_short ack;
    u_char Flags;
    u_short Size;
};

int main(){

    HEADER h1;
    h1.Checksum=55;
    HEADER h2 = h1;
    cout<<h2.Checksum<<endl;

    return 0;
}



