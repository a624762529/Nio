#include "epollnode.h"

EpollNode::EpollNode()
{
}

EpollNode::EpollNode(string ip, int prot, int cfd)
{
    initEpollNode(ip,prot,cfd);
}

void EpollNode::initEpollNode(string ip, int prot, int cfd)
{
    Clie.initInfo(ip,prot,cfd);
    ready_set_infoLen=false;
    canuse=false;
}

void EpollNode::setInfoLen(int len)
{
    this->info_len=len;
    int ret=getStataMemPool(info_len);
    this->Clie.initSockBufMemPool(ret);
    this->Clie.setInfoAllLen(len);
    this->ready_set_infoLen=true;
}

int EpollNode::retRead(int ret)
{
    //即ret==-1 -3移除在epoll树上的节点
    switch (ret) {
    case 0:
        //case为0　如果客户端断开连接
        return 0;
        break;
    case -1:
        //case为-1 读写对端网络原因  等待对端可读
        return -1;
        break;

    case -2:
        //case为-2 如果当前信息读写完毕
        return -2;
        break;

    case -3:
        //如果读写失败
        return -3;
        break;
    default:
        break;
    }
    return ret;
}

int EpollNode::callReadBack()
{
    if(ready_set_infoLen==false)
    {
        //进行　断开链接判断(对读写失败(非网络延迟的原因) 以及读写结束　进行归还资源的处理
        //ret==-2 句柄进行结束处理的操作)

        int ret=retRead(Clie.read_head());
        if(ret==0||ret==-3)
        {
            judeChatOver();
            return ret;
        }
        else if(ret==-2)
            return ret;

        this->setInfoLen(ret);
    }

    int ret=retRead(Clie.nio_read());
    if(ret==0||ret==-3)
    {
        judeChatOver();
        return ret;
    }
    else if(ret==-2)
        return ret;
    readCallBack( this ,this->read_arg );
    if(Clie.judeRWOver())
    {
        //如果读写结束　那就回收超大内存块
        //bug:要是在此处回收超大内存　那在cli中的内存会被归还 如何处理?
        rwOver();
        return -2;
    }
    return 1;
}

void EpollNode::callWritBack()
{
    if(Clie.judeRWOver())
    {
        Clie.doRWOver();
        return;
    }
    writeCallBack(this,this->write_arg);
    Clie.nio_write();
}

void EpollNode::callExpeBack()
{
    expCallBack(this,this->exparg);
}

bool EpollNode::judeSetInfoLen()
{
    if(ready_set_infoLen==false)
    {
        Clie.setInfoAllLen(info_len);
        ready_set_infoLen=true;
        return ready_set_infoLen;
    }
    else
    {
        return ready_set_infoLen;
    }
}

bool EpollNode::judeReadyRead()
{
    if(Clie.judeRWOver())
    {
        Clie.doRWOver();
        return true;
    }
    return false;
}

bool EpollNode::judeReadyWrite()
{
    if(Clie.judeRWOver())
    {
        Clie.doRWOver();
        return true;
    }
    return false;
}

void EpollNode::judeChatOver()
{
    Clie.destory();//清理掉clie的内容

    readCallBack=nullptr;
    read_arg=nullptr;

    writeCallBack=nullptr;
    write_arg=nullptr;

    expCallBack=nullptr;
    exparg=nullptr;

    ready_set_infoLen=false;
    info_len=0;
    canuse=false;
}

bool EpollNode::judeCanUse()
{
    return canuse;
}

void EpollNode::setCanUse()
{
    canuse=true;
}

int EpollNode::getStataMemPool(int len)
{
     int st=len/1024;
     if(st<=5)
         return 1024;
     else if(st<=10)
         return 2*1024;
     else if(st<=20)
         return 3*1024;
     return 3*1024;
}

void EpollNode::setReadInfo
              (function<void (EpollNode *cli,void*)> read_fun,void*readarg)
{
    this->readCallBack=read_fun;
    this->read_arg=readarg;
}

void EpollNode::setWriteInfo
    (function<void (EpollNode *cli,void*)> wri_fun,void* wriarg)
{
    this->writeCallBack=wri_fun;
    this->write_arg=wriarg;
}

void EpollNode::setExpteInfo
    (function<void (EpollNode *cli,void*)> exp_fun,void* exarg)
{
    this->expCallBack=exp_fun;
    this->exparg=exarg;
}

void EpollNode::rwOver()
{
     Clie.doRWOver();
     this->info_len=0;
     this->ready_set_infoLen=false;
}

int EpollNode::getFd()
{
    return Clie.cfd_;
}

EpollNode::~EpollNode()
{

}


int EpollNode::read_cli(char *sock_buf,int len)
{
    //如果当前状态处于读写结束
    if(Clie.judeCurOver())
    {
        //尝试获取首部
        int ret=Clie.read_head();
        //进行　断开链接判断(假设对端网路通畅)
        if(ret==-1)
        {
            return 0;
        }
        this->setInfoLen(ret);
    }
    return Clie.read_cli(sock_buf,len);
}
int EpollNode::write_cli(char *sock_buf,int len)
{
    return Clie.write_cli(sock_buf,len);
}
int EpollNode::nio_read()
{
    return Clie.nio_read();
}
int EpollNode::nio_write()
{
    return Clie.nio_write();
}


void EpollNode::changeNodeEvent(int event)
{

    Epoll *loop=reinterpret_cast<Epoll *>(m_epctrl);
    if(loop==nullptr)
    {
        return;
    }
    loop->changeNodeEvent(this->getFd(),event);
}
