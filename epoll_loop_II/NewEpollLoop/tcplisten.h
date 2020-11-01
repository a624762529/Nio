#ifndef TCPLISTEN_H
#define TCPLISTEN_H
#pragma once
#include"head.h"
#include<string>
using namespace std;
class TcpListen
{
public:
    TcpListen(int port);
    ~TcpListen();
    pair<int, sockaddr_in> acceptConnect();
    void destory();
    int  getFd();
private:
    enum { CanUse };
    int m_magic;
    int lfd;
    sockaddr_in ser;
};

#endif // TCPLISTEN_H
