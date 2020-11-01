#include "epollloop.h"

EpollLoop::EpollLoop(int listen_port):
    m_listen(TcpListen(listen_port))//初始化监听信息
{
    m_magic=-1;
    m_epfd=-1;
    m_read_callback  =nullptr;
    m_write_callback =nullptr;
    m_exp_callback   =nullptr;
    m_accept_cb      =nullptr;
    link_client_qua  =  0    ;
}

void EpollLoop::setCanUse()
{
    if(m_magic==CanUse)
    {
        perror("EpollLoop hava been init");
        throw bad_alloc();
    }
    m_magic=CanUse;
    m_epfd= epoll_create(7000);//创建epoll根节点
    if(m_epfd==-1)
    {
        perror("create epoll root error");
        throw bad_alloc();
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = &m_listen;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD,  m_listen.getFd(), &ev);
    //将监听文件描述符 加入到epoll树 epfd上

    //忽略sigpipe
    signal(SIGPIPE,SIG_IGN);
}


//在epoll上挂上epoll 监听套接字
void EpollLoop::addTcpCommunication(TcpCommunication *com)
{
    if(com==nullptr)
    {
        return;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = com;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD,  com->getFd(), &ev);
    //将监听文件描述符 加入到epoll树 epfd上
    return;
}

void EpollLoop::removeEventFd(TcpCommunication *com)
{
    auto m_iv=m_mp.find(com);
    if(m_iv==m_mp.end())
    {
        perror("mp_error\n");
        return;
    }

    m_mp.erase(m_iv);
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, com->getFd(), NULL);
    com->closeCommunication();
    delete com;
    com=nullptr;
    this->link_client_qua--;
}


//如何做?

void EpollLoop::loop()
{
    //进行loop循环
    while (true)
    {
        int ret = epoll_wait(m_epfd, all, sizeof(all)/sizeof(all[0]), -1);
        if(ret==-1)
        {
            perror("epoll wait error\n");
        }
        HeartAlarm *heart=nullptr;
        for(int i=0;i<ret;i++)
        {
            if(all[i].data.ptr==&m_listen)
            {
                 //客户端 发来链接
                 acceptFd();
                 continue;
            }
            //心跳
            heart=reinterpret_cast<HeartAlarm *>(all[i].data.ptr);
            if(heart!=&m_heart)
            {
                heart=nullptr;
            }
            else
            {
                break;
            }

            if(all[i].events==EPOLLIN)
            {
                //已连接的客户端发来信息
                //执行读操作               
                TcpCommunication *commun=(TcpCommunication *)all[i].data.ptr;
                m_heart.addNode(commun);
                if(commun->callReadCallBack())
                {
                    //文件描述符出问题 或者对端关闭连接
                    removeEventFd(commun);
                }
                if(commun->getStatus()==TcpCommunication::Write)
                {
                    //读取结束 改变文件描述符的监听类型
                    changeTcpComunicationType(commun,EPOLLOUT);
                }
            }
            else if(all[i].events==EPOLLOUT)
            {
                TcpCommunication *commun=(TcpCommunication *)all[i].data.ptr;
                m_heart.addNode(commun);
                if(commun->m_in_heart_que==false)
                {
                    perror("异常 文件描述符不在epoll树上\n");
                }
                if(commun->callWriteCallBack())
                {
                    removeEventFd(commun);
                }
                if(commun->getStatus()==TcpCommunication::Read)
                {
                    //写入结束 讲文件描述符转换为可读的状态
                    changeTcpComunicationType(commun,EPOLLIN);
                }
            }
        }
        if(heart!=nullptr)
        {
            auto ret= heart->timeout_Act(m_mp);
            if(ret.size()!=0)
            {
                cout<<"因为心跳终止的 连接"<<ret.size()<<endl;
            }
            for(auto it=ret.begin();it!=ret.end();it++)
            {
                removeEventFd( *it);
            }
        }

    }
}

void EpollLoop::changeTcpComunicationType
                (TcpCommunication *com,int type)
{
    struct epoll_event ev;
    ev.events = type;
    ev.data.ptr = com;
    epoll_ctl(m_epfd, EPOLL_CTL_MOD, com->getFd(), &ev);
}

void EpollLoop::setReadcb(function<void(char*,int,void *,TcpCommunication *)> read_callback)
{
    m_read_callback=read_callback;
}


void EpollLoop::setWritecb(function<void (void *arg,TcpCommunication *tcpCon)> write_callback)
{
    m_write_callback=write_callback;
}


void EpollLoop::setExpcb(function<void (char*,int,void*)>  exp_callback)
{
    m_exp_callback=exp_callback;
}

void EpollLoop::acceptFd()
{
    pair<int,sockaddr_in> ret=
            m_listen.acceptConnect();
    TcpCommunication *commun=new TcpCommunication
            (ret.first,ret.second);
    addTcpCommunication(commun);

    commun->setExpCallBack  (m_exp_callback,nullptr);
    commun->setReadCallBack (m_read_callback,nullptr);
    commun->setWriteCallBack(m_write_callback,nullptr);


    commun->changeStatus(TcpCommunication::Read);
    commun->m_tag=0;
    commun->m_in_heart_que=false;

    m_heart.addNode(commun);
    link_client_qua++;
    m_mp.insert(map<TcpCommunication*,bool>::value_type(commun,true));

    if(m_accept_cb)
        m_accept_cb(commun);
}

EpollLoop::~EpollLoop()
{
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_listen.getFd(), NULL);
    close(m_epfd);
    m_listen.destory();
}


void EpollLoop::setAcceptcb(function<void (TcpCommunication*)>  accept_cb)
{
    m_accept_cb=accept_cb;
}


void EpollLoop::initHeartTime(int time)
{
    //初始化相应的时间
    if(time<4)
    {
        time=4;
    }
    m_heart.initAlarm(time/4);
    //将超时机制 挂在epoll上
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = &m_heart;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_heart.getReadFd(), &ev);
}

void EpollLoop::startAlarm()
{
    m_heart.start();
}
