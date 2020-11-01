#ifndef EPOLLNODE_H
#define EPOLLNODE_H
#include"head.h"
#include"tcp_client.h"

#include"epollloop.h"
#include"epollnode.h"
#include<functional>
//决策通讯还是连接 那是Epoll的事情

class EpollNode
{
public:
    //friend class Epoll;
    EpollNode();
    EpollNode(string ip, int prot, int cfd);
    ~EpollNode();
    
public:
    //函数调用
    int  callReadBack();
    void callWritBack();
    void callExpeBack();
    void judeChatOver();
public:
    void setReadInfo(function<void (EpollNode *cli,void*)>,void*);
    void setWriteInfo(function<void (EpollNode *cli,void*)>,void*);
    void setExpteInfo(function<void (EpollNode *cli,void*)>,void*);
public:
    void initEpollNode(string ip,int prot,int cfd);
    void setInfoLen(int len);//设置信息长度
    bool judeCanUse();
    void setCanUse();
    void rwOver();         //设置读写结束
    int  getFd();

    void changeNodeEvent(int event);
//10.27 思路　让用户 操作epollnode部分
//           用户使用之后不容改变? 思虑一下吧
public:
    int read_cli(char *sock_buf,int len);
    int write_cli(char *sock_buf,int len);
    int nio_read();
    int nio_write();
private:
    bool judeSetInfoLen(); //是否已经设置了信息的长度
    bool judeReadyRead();  //决策是否已经读取结束
    bool judeReadyWrite(); //决策是否已经写结束
    bool isSetingInfoLen();//判断是否已经设置了infolen
private:
    int getStataMemPool(int len);
    int retRead(int ret);
private:
    bool ready_set_infoLen; //是否已经设置了通讯信息的的长度
    int  info_len;        //通讯信息的长度
    bool canuse;          //是否处于被使用的状态
private:
    TcpClient Clie;
    function<void (EpollNode *cli,void*)> readCallBack;
    void* read_arg;
    function<void (EpollNode *cli,void*)> writeCallBack;
    void* write_arg;
    function<void (EpollNode *cli,void*)> expCallBack;
    void* exparg;
public:
    void *m_epctrl=nullptr;
};

#endif // EPOLLNODE_H
