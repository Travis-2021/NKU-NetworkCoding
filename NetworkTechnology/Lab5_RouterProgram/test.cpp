#include<iostream>
using namespace std;
#include <string>
#include<winsock2.h>

#pragma pack(1)
struct test{
//    unsigned a:32;
//    unsigned b:4;
//    unsigned c:29;
    char a;
    unsigned b:8;
};

#pragma pack()

int main(){
    int arr[6];
    int* p = arr;
    cout<<sizeof(p)<<endl;

    system("pause");
    return 0;
}