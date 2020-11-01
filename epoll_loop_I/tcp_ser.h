#ifndef TCP_SER_H
#define TCP_SER_H


#include"epollnode.h"
#include"head.h"
#include"sock_buff.h"
#include"tcp_client.h"

class EpollNode;
class TcpSer
{
public:
    TcpSer();
    TcpSer(int prot);
    ~TcpSer();
    void init_Ser();
    pair<int,sockaddr_in> nio_accept();//accept
    int getListenFd();
    void destory();
    bool operator<(const TcpSer& com)const &;
    void setReadcb(function<void (EpollNode *cli,void*)> rdcb,void* read_arg);
    void setWritcb(function<void (EpollNode *cli,void*)> wrcb,void* writ_arg);
    void setExeccb(function<void (EpollNode *cli,void*)> excb,void* exep_arg);

    bool operator==(const TcpSer &ser);
private:
    int nio_listen();//设置监听
    int nio_creat_sock();//创建文件描述符
    int nio_bind();//绑定ip和端口
    void set_nonblock();
private:

    void* paintself;//指向自己的指针 待该对象使用结束之后 将该指针 归还给内存池 如此罢了
    int listen_fd;
    int prot;

    //SockBuf作为通讯的借口 必须待将其中的数据处理完毕
    //例如 将读出的数据处理完 才能写入数据
public:
    function<void (EpollNode *cli,void*)> m_readcb;
    void *arg_read;
    function<void (EpollNode *cli,void*)> m_writcb;
    void *arg_write;
    function<void (EpollNode *cli,void*)> m_expecb;
    void *arg_exp;
};

#endif // TCP_SER_H
