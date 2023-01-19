
#ifndef TWOPEOPLE_CHATROOM_SUPPORTFUNC_H
#define TWOPEOPLE_CHATROOM_SUPPORTFUNC_H

#include <winsock.h>


void getNowtime();
void ReceiveThread(SOCKET c);
void SendThread(SOCKET c);


#endif //TWOPEOPLE_CHATROOM_SUPPORTFUNC_H
