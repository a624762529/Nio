#ifndef EPOLLLOOP_H
#define EPOLLLOOP_H
#include"heartalarm.h"
#include"head.h"
#include"epollnode.h"
#include"tcp_ser.h"
#include"tcp_client.h"
#include"tcp_buf.h"

#include<iostream>
#include<map>
using namespace std;

class EpollNode;
class HeartAlarm;
class TcpSer;

class Epoll
{
public:
    Epoll();
    Epoll(int size);
    ~Epoll();
    void loop();                                                //事件循环
                                                                //loop的时候注意监听和通讯文件描述符的区别处理
    void addNode(shared_ptr<EpollNode> node);
    void rmNode (shared_ptr<EpollNode> node);
    void changeNodeEvent(shared_ptr<EpollNode> node,int event);//改变文件的监听信息
    void changeNodeEvent(int fd,int event);                    //改变文件的监听信息
    void setListenSock(shared_ptr<TcpSer> node);               //设置监听信息
    void setAlarm(int time);
    void setDefultReadcb(function<void (EpollNode *cli,void*)> rdcb,
                         void* read_arg);
    void setDefultWritcb(function<void (EpollNode *cli,void*)> wrcb,
                         void* writ_arg);
    void setDefultExeccb(function<void (EpollNode *cli,void*)> excb,
                         void* exep_arg);
    void addToThreadPool(shared_ptr<EpollNode> node);          //将该节点加入到内存池中
protected:
    void createEpollRoot(int size);                            //创建epoll根节点
    void destory();
    int  findIndex();
    void setCallBack(shared_ptr<EpollNode> node,shared_ptr<TcpSer>  m_alarm);
    void startListenHeart();
protected:
    bool start;
    shared_ptr<HeartAlarm>  m_alarm;
    struct epoll_event m_listenev;                           //监听事件描述节点  永远处于可读的状态
    shared_ptr<TcpSer>             m_lis;
    int m_epfd;                                              //epoll根节点
    int m_heart_epfd;
    struct epoll_event *m_ev_arry;                           //事件描述汇总
    bool *m_lim_arry;                                        //ev_arry的映射信息　利用之判断ev_arry中有几个可用节点 0代表可用　１代表不可用
    int m_limit;                                             //节点个数限制

    shared_ptr<EpollNode> *chargenode;
    map<int,shared_ptr<EpollNode>> m_shmap;                  //文件描述符与EpollNode的映射
    int m_readnum;                                           //正在读的事件个数
    int m_writenum;                                          //正在写的事件个数
    //如果tcpser没有设置默认函数　以及函数参数　那就使用这套函数
    function<void (EpollNode *cli,void*)> m_defaltreadcallback;
    void *arg_read;
    function<void (EpollNode *cli,void*)> m_defaltwritecallback;
    void *arg_write;
    function<void (EpollNode *cli,void*)> m_defaltexptcallback;
    void *arg_exp;
};
#endif // EPOLLLOOP_H
