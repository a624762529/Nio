#ifndef EPOLLLOOPI_H
#define EPOLLLOOPI_H
#include"epollloop.h"
#include"head.h"
class EpollLoop:public Epoll
{
public:
    EpollLoop();
    ~EpollLoop();
public:

    void addSer(shared_ptr<TcpSer> add_addr);
    void removeSer(shared_ptr<TcpSer> add_addr);
    void loop();
    void changeNodeEvent(shared_ptr<EpollNode> node,int event); 
    void addToThreadPool(shared_ptr<EpollNode> node); 
private:
    int epollAccept(epoll_event ev);
    vector<shared_ptr<TcpSer>> vec_ser;
    vector<epoll_event>        vec_listen;

};

#endif // EPOLLLOOPI_H
