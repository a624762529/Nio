#ifndef TCP_BUF_H
#define TCP_BUF_H
#include"head.h"
using namespace std;

//我希望tcpclient是具备弹性的 可以进行心跳的发送 也可以不进行心跳的发送
class Client
{
public:
    Client(string ip,int prot);
    void nio_connect();
    int nio_read(char *buf,int len);
    int nio_write(char *buf,int len);
    void set_nonblock();
private:
    //(int cfd,string ip_,int prot_)
    string ip_;
    int prot_;
    int cfd;
};


#endif // TCP_BUF_H
