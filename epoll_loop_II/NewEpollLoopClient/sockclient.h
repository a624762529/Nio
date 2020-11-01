#ifndef SOCKCLIENT_H
#define SOCKCLIENT_H
#include"head.h"
#include<mutex>
#include<thread>
#include<signal.h>
#include<sys/signal.h>

using namespace std;



class SockClient
{
public:
    SockClient();
    ~SockClient();
    SockClient(string ip,int prot);
    int   readInfo(char *buf,int len);
    int   writeInfo(char *buf,int len);
    void  setIp(string ip);
    void  setProt(int prot);
    void  destory();
    int   connectToHost();
    int   stableRecv(char *buf,int len);
    int    createSocket(string ip,int prot);
private:


    void   setNoBlock();
    void   setBlock();
    int    connectToSer();//非阻塞connect

public:
    void startAlarm();
    void start();

private:
    bool m_starAlarm;
    int cfd;
    sockaddr_in addr;
};

#endif // SOCKCLIENT_H
