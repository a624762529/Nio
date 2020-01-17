#include "epollloopi.h"

EpollLoop::EpollLoop()
{

}

void EpollLoop::changeNodeEvent(shared_ptr<EpollNode> node,int event)
{

    switch(event)
    {
    case EPOLLIN:
        Epoll::changeNodeEvent(node,EPOLLOUT);
        break;

    case EPOLLOUT:
        Epoll::changeNodeEvent(node,EPOLLIN);
        break;

    case EPOLLPRI:
        Epoll::changeNodeEvent(node,EPOLLPRI);
        break;
    }
    return;
}

void EpollLoop::addSer(shared_ptr<TcpSer> add_addr)
{
    vec_ser.push_back(add_addr);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr=reinterpret_cast<void*>(&add_addr); 
    int ret=epoll_ctl(m_epfd, EPOLL_CTL_ADD,
                      add_addr->getListenFd(), &ev);
    if(ret==-1)
    {
        perror("epoll_ctl err");
        throw bad_alloc();
    }
}

void EpollLoop::removeSer(shared_ptr<TcpSer> add_addr)
{

    for(vector<shared_ptr<TcpSer>>::iterator it=vec_ser.begin();
        it!=vec_ser.end();it++)
    {
        if(it!=vec_ser.end())
        {
            epoll_ctl(m_epfd, EPOLL_CTL_DEL, (*it)->getListenFd(), NULL);;
            (*it)->destory();
            vec_ser.erase(it);
        }
    }
}

EpollLoop::~EpollLoop()
{
    for(vector<shared_ptr<TcpSer>>::iterator it=vec_ser.begin();
        it!=vec_ser.end();it++)
    {
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, (*it)->getListenFd(), NULL);;
        (*it)->destory();
    }
}

void EpollLoop::loop()
{
    while (true)
    {
        //信息现在锁在其中出不来
        int ret=epoll_wait(m_epfd,m_ev_arry,m_limit,-1);
        for(int  i=0;i<ret;i++)
        {
            if(m_alarm)
            {
                if(m_ev_arry[i].data.ptr==&m_alarm)
                {
                    m_alarm->moveNext();
                    m_alarm->readChar();
                    continue;
                }
            }

            int ret_=epollAccept(m_ev_arry[i]);
            if(ret_==1||ret_==-1)
            {
                //链接成功　或者失败(确认是监听节点触发的事件)
                continue;
            }

            shared_ptr<EpollNode> act= *((shared_ptr<EpollNode> *)
                    m_ev_arry[i].data.ptr);
            if(m_ev_arry[i].events==EPOLLPRI)
            {
                //心跳发送过来
                act->callExpeBack();
            }
            else if(m_ev_arry[i].events==EPOLLIN)
            {
                int ret=act->callReadBack();
                if(ret==-3||ret==0)
                {
                    //移除该节点
                    this->rmNode(act);
                }
            }
            else if(m_ev_arry[i].events==EPOLLOUT)
            {
                act->callWritBack();
            }
        }

    }
}

int EpollLoop::epollAccept(epoll_event ev)
{

    for(vector<shared_ptr<TcpSer>>::iterator it=vec_ser.begin();
        it!=vec_ser.end();it++)
    {
        //如果　当前ev的(void*)指针指向的内容是监听文件描述符　那就进行链接操作就行
        shared_ptr<TcpSer> *sh=(shared_ptr<TcpSer>*)
                (ev.data.ptr);
        if((*sh)==(*it))
        {
            pair<int,sockaddr_in> ret= (*it)->nio_accept();
            //accept失败 是可被接受的失败
            int prot=ntohs(ret.second.sin_port);
            string ip=inet_ntoa(ret.second.sin_addr);

            if(ret.first!=-1)
            {
                shared_ptr<EpollNode> share_node(new EpollNode
                                                 (ip,prot,ret.first));
                //设置默认的cb
                share_node->m_epctrl=(void*)this;
                setCallBack(share_node,(*it));
                addNode(share_node);
                return 1;
            }
            return -1;
        }
    }
    return 0;
}

void EpollLoop::addToThreadPool(shared_ptr<EpollNode> node)
{
    //改变节点的监听事件
    this->changeNodeEvent(node,EPOLLPRI);
    //将节点加入到线程池
}
