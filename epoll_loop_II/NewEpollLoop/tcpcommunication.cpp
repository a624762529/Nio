#include "tcpcommunication.h"

TcpCommunication::TcpCommunication(int cfd,sockaddr_in addr)
{
    this->m_cfd=cfd;
    this->m_addr=addr;
    m_status=Status::Free;
    m_tag=0;
    m_in_heart_que=false;
}

void TcpCommunication::setReadCallBack
      (function<void(char*,int,void *,TcpCommunication *)> read_cb,void *read_arg)
{
    m_read_callback=read_cb;
    m_read_arg=read_arg;
}

void TcpCommunication::setWriteCallBack
     (function<void (void *arg,TcpCommunication *tcpCon)> write_cb ,void *write_arg)
{
    m_write_callback=write_cb;
    m_write_arg=write_arg;
}

void TcpCommunication::setExpCallBack
     (function<void (char*,int,void *)>  exp_cb,void *exp_arg)
{
    m_exp_callback=exp_cb;
    m_exp_arg=exp_arg;
}


//返回true说明通讯结束 对端关闭连接
bool TcpCommunication::callReadCallBack()
{
    if(m_status!=Status::Read)
    {
        cout<<"call read back error"<<endl;
        throw bad_alloc();
        return false;
    }
    if(m_read_callback==nullptr)
    {
        perror("not set read callback");
        throw bad_alloc();
        return false;
    }
    if(m_have_set_read_len==false)
    {
        //未设置buf的长度  读取头部的时候可能发生客户端关闭连接 或者发生了不可更改的错误
        int len=readHeadLen();
        if(len==-2)
        {
            //read error  让客户端关闭连接
            return true;
        }
        else if(len==-1)
        {
            return false;
        }
        else if(len==0)
        {
             //closeCommunication();
             return true;
        }
        else
        {
            m_have_set_read_len=true;
            if(m_read_buf!=nullptr)
            {
                free(m_read_buf);
            }
            m_read_buf =new char[len];
            m_ready_read_len=0;    //已经读取的长度
            m_max_read_len=len;    //发送的包的内容
        }
    }

    int ret=readInfo();
    if(ret==0)
    {
        //客户端关闭链接
        //closeCommunication();
        return true;
    }
    if(isReadOver())//如果信息读取完毕
    {
        m_read_callback(m_read_buf,m_max_read_len,m_read_arg,this);
        doReadOver();
    }
    return false;
}

bool TcpCommunication::callWriteCallBack()
{
    if(m_status!=Status::Write)
    {
        cout<<"call write back error"<<endl;
        throw bad_alloc();
    }
    if(m_write_callback==nullptr)
    {
        perror("not set read callback");
        throw bad_alloc();
    }
    m_write_callback(m_write_arg,this);

    int ret=writeInfo();
    if(ret==-1)
    {
        return false;
    }
    else if(ret==-2)
    {
        return true;
    }
    if(isWriteOver())
    {
        doWriteOver();
    }
    return false;
}

void TcpCommunication::callExpCallBack()
{
    if(m_exp_callback==nullptr)
    {
        perror("not set read callback");
        throw bad_alloc();
    }
}

int TcpCommunication::readInfo()
{
    //将相应的数据读取到buf中
    int ret=read(m_cfd,&m_read_buf[m_ready_read_len],
                 m_max_read_len-m_ready_read_len);
    if(ret==0)
    {
        //关闭通讯文件描述符
        //..
        return ret;
    }
    else if(ret>0)
    {
        m_ready_read_len+=ret;
        return ret;
    }
    else
    {
        if(errno==EAGAIN)
        {
            return -1;
        }
        cout<<"read error"<<__LINE__<<endl;
        throw bad_alloc();
    }

}

int TcpCommunication::writeInfo()
{
    int ret=write(m_cfd,&m_write_buf[m_ready_write_len],
                  m_max_write_len-m_ready_write_len);
    if(ret>0)
    {
        m_ready_write_len+=ret;
        return ret;
    }
    else
    {
        if(errno!=EWOULDBLOCK)
        {
            perror("read err");
            cout<<__LINE__<<__FUNCTION__<<endl;
            return -2;
        }
        return -1;
    }
    return -1;
}

void TcpCommunication::closeCommunication()
{
    //cout<<"client dis connect"<<endl;
    close(m_cfd);
    doReadOver();
    doWriteOver();
    m_read_callback(nullptr,0,m_read_arg,nullptr);
}

int  TcpCommunication::readHeadLen()
{
    int num=0;
    int ret=recv(m_cfd,&num,4,MSG_WAITALL);
    if(ret==-1)
    {
        if(errno==EAGAIN)
        {
            return ret;
        }
        return -2;
    }
    return num;
}

bool TcpCommunication::isReadOver()
{
    return m_ready_read_len==m_max_read_len;
}

void TcpCommunication::doReadOver()
{
    if(m_read_buf)
    {
        delete []m_read_buf;
        m_read_buf=nullptr;
    }
    m_ready_read_len=0;
    m_max_read_len=0;
    m_have_set_read_len=false;
}

bool TcpCommunication::isWriteOver()
{
    return m_ready_write_len==m_max_write_len;
}

void TcpCommunication::doWriteOver()
{
    if(m_write_buf)
    {
        delete []m_write_buf;
        m_write_buf=nullptr;
    }
    m_ready_write_len=0;
    m_max_write_len=4;
}

void TcpCommunication::addInfoToWrite(char *buf,int len)
{
    InfoPack *p=reinterpret_cast<InfoPack*>(m_write_buf);
    memcpy(&p->data[m_ready_write_len],
           buf,len);
    m_max_write_len+=len;
}

void TcpCommunication::setInfoLen(int len)
{
    doWriteOver();
    m_have_set_write_len=true;
    m_write_buf=new char[len+4];

    InfoPack *p=reinterpret_cast<InfoPack *>(m_write_buf);
    p->len=len;
}

int TcpCommunication::getStatus()
{
    return m_status;
}

void TcpCommunication::changeStatus(int status)
{
    m_status=status;
}

int  TcpCommunication::getFd()
{
    return m_cfd;
}
