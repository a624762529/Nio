#ifndef EPOLLLOOP_H
#define EPOLLLOOP_H

#include"head.h"
#include"tcplisten.h"
#include"tcpcommunication.h"
#include"heartalarm.h"
#include<iostream>
#include<map>
using namespace std;



class EpollLoop
{
public:
    EpollLoop(int listen_port);
    ~EpollLoop();
    void loop();
    void setCanUse          ();
    void addListen          ();
    void removeEventFd      (TcpCommunication *com);
    void addTcpCommunication(TcpCommunication *com);
    void changeTcpComunicationType(TcpCommunication *com,int type);
    void setReadcb  (function<void (char* arry,int len,void *arg,TcpCommunication *tcpCon)>  read_callback);
    void setWritecb (function<void (void *arg,TcpCommunication *tcpCon)> write_callback);
    void setExpcb   (function<void (char*,int,void*)>  exp_callback);
    void setAcceptcb(function<void (TcpCommunication*)>  accept_cb);
    void initHeartTime(int time);    //初始化心跳时间
    void startAlarm();
private:
    void acceptFd();
private:
    enum {CanUse=11};
    int m_magic=-1;
    int m_epfd=-1;
    struct epoll_event all[2000];//事件集合
    TcpListen          m_listen; //监听
    int                m_listen_port=0;
    function<void(char*,int,void *,TcpCommunication *)>  m_read_callback;
    function<void (void *,TcpCommunication *)>           m_write_callback;
    function<void (char*,int,void*)>                     m_exp_callback;
    function<void (TcpCommunication*)>                   m_accept_cb;
    HeartAlarm                                           m_heart;

    atomic<int> link_client_qua;

    map<TcpCommunication*,bool> m_mp;
    //万般无奈出此下策
    //因为....  有两个地方 需要清理
    //第一个是 用户主动断开链接
    //不用智能指针 搞得很被动
    //第二种是 清理时间队列的时候
};

#endif // EPOLLLOOP_H
