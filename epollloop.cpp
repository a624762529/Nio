#include "epollloop.h"

Epoll::Epoll():Epoll(1024)
{}

Epoll::Epoll(int size)
{
    //初始化loop节点
    start=true;
    createEpollRoot(size);
    //初始化事件信息
    m_ev_arry=new epoll_event[size];
    //更改epoll节点的标志位信息
    m_limit=size;
    m_lim_arry=new bool[size];
    //将其中的信息全部初始化false
    memset(m_lim_arry,0,sizeof(m_lim_arry));
    //初始化epollnode信息
    chargenode=new shared_ptr<EpollNode>[size];
    m_readnum=0;
    m_writenum=0;

    thread th(&Epoll::startListenHeart,this);
    th.detach();
}

Epoll::~Epoll()
{
    start=false;

    for(int i=0;i<m_limit;i++)
    {
        if(chargenode[i])
        {
            rmNode(chargenode[i]);
        }
    }

    m_limit=0;
    m_readnum=0;
    m_writenum=0;

    if(m_alarm)
    {
        //将alarm设置为
    }


    delete[] chargenode;
    delete[] m_lim_arry;
    delete[] m_ev_arry;

}

void Epoll::setListenSock(shared_ptr<TcpSer> node)
{
    m_lis=node;
    m_listenev.events = EPOLLIN;
    m_listenev.data.ptr=reinterpret_cast<void*>(&m_lis);
    int ret=epoll_ctl(m_epfd, EPOLL_CTL_ADD,
                      m_lis->getListenFd(), &m_listenev);
    if(ret==-1)
    {
        perror("err");
    }
}

void Epoll::createEpollRoot(int size)
{
    m_epfd=epoll_create(size);
    //创建EPOLL根节点
    if(m_epfd==-1)
    {
        perror("create root err");
        throw bad_alloc();
    }

    m_heart_epfd=epoll_create(size);
    if(m_heart_epfd==-1)
    {
        perror("create root err");
        throw bad_alloc();
    }
}
//addNode的时候　还应根据是否加载心跳动态选择监听异常文件描述符
void Epoll::addNode(shared_ptr<EpollNode> node)
{
    int ret=findIndex();
    if(ret==-1)
    {
        perror("链接人数过多");
        return;
    }
    chargenode[ret]=node;
    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.ptr=(void*)&(chargenode[ret]);
    epoll_ctl(m_epfd,EPOLL_CTL_ADD,node->getFd(),&ev);

    ev.events=EPOLLPRI;
    ev.data.ptr=(void*)&(chargenode[ret]);
    epoll_ctl(m_heart_epfd,EPOLL_CTL_ADD,node->getFd(),&ev);
    //将节点插入到映射组
    node->m_epctrl=reinterpret_cast<void*>(this);
    m_shmap.insert(pair<int,shared_ptr<EpollNode>>(node->getFd(),node));
}

void Epoll::rmNode (shared_ptr<EpollNode> node)
{
    int fd=node->getFd();
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
    epoll_ctl(m_heart_epfd, EPOLL_CTL_DEL, fd, NULL);
    node.reset();
    //移除节点信息
    auto rm_mp=m_shmap.find(fd);
    if(rm_mp!=m_shmap.end())
    {
        m_shmap.erase(rm_mp);
    }
}

int Epoll::findIndex()
{
    for(int i=0;i<m_limit;i++)
    {
        if(chargenode[i]==nullptr)
        {
            return i;
        }
    }
    return -1;
}

void Epoll::changeNodeEvent(shared_ptr<EpollNode> node,int event)
{
    int fd=node->getFd();
    if(event==EPOLLIN||event==EPOLLOUT)
    {
        struct epoll_event ev;
        ev.events=event;
        ev.data.ptr=(void*)&node;
        epoll_ctl(m_epfd,EPOLL_CTL_MOD,fd,&ev);
    }
}

void Epoll::changeNodeEvent(int fd,int event)
{

    auto it=m_shmap.find(fd);
    if(it==m_shmap.end())
    {
        perror("error");
        return;
    }
    shared_ptr<EpollNode> node=(*it).second;

    if(event==EPOLLIN||event==EPOLLOUT)
    {
        struct epoll_event ev;
        ev.events=event;
        ev.data.ptr=(void*)&node;
        epoll_ctl(m_epfd,EPOLL_CTL_MOD,fd,&ev);
    }
}

void Epoll::loop()
{
    //事件循环
    while (true)
    {
        int ret=epoll_wait(m_epfd,m_ev_arry,m_limit,-1);
        for(int  i=0;i<ret;i++)
        {
            //默认null　是心跳触发
            if(m_alarm)
            {
                if(m_ev_arry[i].data.ptr==&m_alarm)
                {
                    m_alarm->moveNext();
                    m_alarm->readChar();
                    continue;
                }
            }
            //这块写死了
            if(m_ev_arry[i].data.ptr==&m_lis)
            {
                pair<int,sockaddr_in> ret= m_lis->nio_accept();
                int prot=ntohs(ret.second.sin_port);
                string ip=inet_ntoa(ret.second.sin_addr);

                if(ret.first!=-1)
                {
                    shared_ptr<EpollNode> share_node(new EpollNode
                                                     (ip,prot,ret.first));
                    addNode(share_node);
                    //设置默认的cb
                    setCallBack(share_node,m_lis);
                    continue;
                }
            }
            shared_ptr<EpollNode> act= *((shared_ptr<EpollNode> *)
                    m_ev_arry[i].data.ptr);
            //如果是关于链接的事件
            //心跳发过来了
            if(m_ev_arry[i].events==EPOLLPRI)
            {
                //心跳发送过来
                act->callExpeBack();
            }
            else if(m_ev_arry[i].events==EPOLLIN)
            {
                int ret=act->callReadBack();
                if(ret==0||ret==-1)
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


void Epoll::setAlarm(int time)
{
    if(m_alarm)
    {
        m_alarm->setPerHeart(time);
        return;
    }

    m_alarm=shared_ptr<HeartAlarm>(new HeartAlarm(time));

    m_alarm->setPerHeart(time);
    m_alarm->setCanUse();//启动写线程
    int heart_fd=m_alarm->getAlarmFd();
    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.ptr=reinterpret_cast<void*>(&m_alarm);

    int ret=epoll_ctl(m_epfd,EPOLL_CTL_ADD,heart_fd,&ev);
    if(ret==-1)
    {
        perror("epoll add alarm err:");
    }

    return;
}


void Epoll::setDefultReadcb(function<void (EpollNode *cli,void*)> rdcb,void* read_arg)
{
    m_defaltreadcallback=rdcb;
    arg_read=read_arg;
}

void Epoll::setDefultWritcb(function<void (EpollNode *cli,void*)> wrcb,void* writ_arg)
{
    m_defaltwritecallback=wrcb;
    arg_write=writ_arg;
}

void Epoll::setDefultExeccb(function<void (EpollNode *cli,void*)> excb,void* exep_arg)
{
    m_defaltexptcallback=excb;
    arg_exp=exep_arg;
}


void Epoll::setCallBack(shared_ptr<EpollNode> shnd,shared_ptr<TcpSer>  arg_ser)
{
    //如果ser中有默认的cb　那就用ser的　没有那就用EpollLoopII
    if(arg_ser->m_readcb)
    {
        shnd->setReadInfo(arg_ser->m_readcb,arg_ser->arg_read);
    }
    else
    {
        shnd->setExpteInfo(m_defaltreadcallback,arg_read);
    }

    if(arg_ser->m_writcb)
    {
        shnd->setWriteInfo(arg_ser->m_writcb,arg_ser->arg_write);
    }
    else
    {
        shnd->setWriteInfo(m_defaltwritecallback,arg_write);
    }

    if(arg_ser->m_expecb)
    {
        shnd->setExpteInfo(arg_ser->m_expecb,arg_ser->arg_exp);
    }
    else
    {
        shnd->setExpteInfo(m_defaltexptcallback,arg_exp);
    }

}

void Epoll::startListenHeart()
{
    struct epoll_event *all =new epoll_event[m_limit];
    char read[20];
    cout<<"listen_heart start"<<endl;
    while (start)
    {

        int ret = epoll_wait(m_heart_epfd, all, m_limit, -1);
        for(int i=0; i<ret; ++i)
        {
            shared_ptr<EpollNode> act= *((shared_ptr<EpollNode> *)
                    all[i].data.ptr);
            if(all[i].events & EPOLLPRI)
            {

                int ret=recv(act->getFd(),&read,20,MSG_OOB);
                cout<<ret<<endl;
//                if(ret==-1)
//                {
//                    if(errno!=EWOULDBLOCK)
//                    {
//                        perror("带外数据读取异常");
//                        exit(0);
//                    }
//                }

                {
                    //心跳发送过来
                    //act->callExpeBack();
                    //跟新心跳中的节点
                    //m_alarm->updateIndex(act);
                }
            }
        }
    }
    cout<<"listen_heart end"<<endl;
    delete all;
}

