#ifndef TCPCOMMUNICATION_H
#define TCPCOMMUNICATION_H
#include"head.h"
using namespace std;

struct InfoPack
{
    int len;
    char data[1];
};


class TcpCommunication
{
public:
    TcpCommunication(int cfd,sockaddr_in addr);
    void closeCommunication();
    int  getStatus();
    void changeStatus(int status);
    int  getFd();
public:
    void setReadCallBack (function<void(char*,int,void *,TcpCommunication *)>read_cb,
                          void *read_arg =nullptr);
    void setWriteCallBack(function<void (void *arg,TcpCommunication *tcpCon)> write_cb,
                          void *write_arg);
    void setExpCallBack  (function<void (char*,int,void*)> exp_cb,
                          void *exp_arg  =nullptr);
public:
    bool callReadCallBack();
    bool callWriteCallBack();
    void callExpCallBack();
public:
    int  readInfo ();
    int  writeInfo();

    void addInfoToWrite(char *buf,int len);
    void setInfoLen(int len);
private:
    int  readHeadLen();
    bool isReadOver();
    void doReadOver();
    bool isWriteOver();
    void doWriteOver();

private:
    int                         m_cfd;
    sockaddr_in                 m_addr;
    function<void(char*,int,void *,TcpCommunication *)>  m_read_callback;

    void *m_read_arg=nullptr;  //call_read_back的参数
    char *m_read_buf=nullptr;  //readbuf的内存
    int   m_ready_read_len=0;  //已经读取的长度
    int   m_max_read_len=0;    //发送的包的内容
    bool  m_have_set_read_len=false; //是否已经设置了buf长度

    function<void (void *arg,TcpCommunication *tcpCon)>  m_write_callback;
    void *m_write_arg=nullptr; //call_write_back的参数
    char *m_write_buf=nullptr; //writebuf的内存
    int   m_ready_write_len=0; //已经写入的长度
    int   m_max_write_len=0;   //要发送的包的长度
    bool  m_have_set_write_len=false;

    function<void (char*,int,void*)>  m_exp_callback;//异常函数
    void *m_exp_arg=nullptr;
    char  m_exp_buf[10]{0};
public:
    enum Status
    {
        Read,Write,Free
    };
    int  m_status=Free;

    int  m_tag;       //标签 在缓冲队列上接收的数据
    bool m_in_heart_que;  //是否 在epoll树上
};

#endif // TCPCOMMUNICATION_H
