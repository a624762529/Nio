#ifndef SOCKETARRY_H
#define SOCKETARRY_H
#include"head.h"
using namespace std;
struct SocketArry
{
    string read_info;       //将从客户端读取的内容放到这个里面
    int    all_read_info=0; //一共到读取的信息的长度

    string write_info;      //记录要写给客户端的内容
    int    all_write_info=0;//一共要写多少信息
};

#endif // SOCKETARRY_H
