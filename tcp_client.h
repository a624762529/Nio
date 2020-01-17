#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include"head.h"
#include"sock_buff.h"
#include"mem_poll.h"

/*
 * tcpclient作为epoll树的通讯节点
 *     Client作为和服务器通讯的要道
 *     客户端给服务器发送信息的时候
 *         必须先发送一个信息首部
 *              包含通讯信息的概要 比如说 文件路径 信息大小 之类的
 *              tcpclient收到报文之后 1.决定要不要向超大内存块申请内存
 *                                  2.记录文件长度
 *                                  3.在读取文件描述符的内容的时候 注意文件长度的变化
 *                                    读取完毕之后就归还超大内存块的内容
 *                                  4.在tcpClient上面设置一个游标记录读取的位置
 *                                      （游标主要记录上次读取的位置是处于
 *                                        从内存池申请的内存还是从超大内存块申请的内存）
 *
*/
#define InMemPool true
#define InBigBuf  false
#define cancontwr true
#define cannotcontwr  false
/*
    关于辅助数据和TcpClient的配合问题
        tcpclient 有一下几个功能：
                   客户函数所使用的读写功能
                   当文件描述符处于可读可写的状态的时候 nioread
                                                  niowrite（当描述符处于可写的状态）
      cliread的时候：
                会移动非空闲索引 读取完mempool中的内存块的时候 将cur置为false（如果已经申请到了内存的话）
      nioread的是偶
                根据cur的状态进行写入 如果cur处于mempool中 那就先写入到mempool中
                                   要是cur处于bigbuf中  那就先往bigbuf中进行写入

      cliwrite的时候:(客户往sockbuf中写入数据)
                                   根据cur的状态进行写入 cur为true
                                   那就将它写入到 mempool中cur=false将它写入到bigbuf中
      niowrite的时候 cur在哪 就将哪块的内容写入到文件描述符中
*/
class AssistInfo
{
public:
    bool read_cur;  //cur为真 处于内存池中申请的内存中
                     //   为假 处于超大内存块上 数据读取完毕之后自动切换
    bool write_cur;
    int fd_len;//已经在文件描述符上读取的信息长度

    bool can_read;      //可以继续从文件描述符中去读数据
    bool can_write;     //可以继续从文件描述符中写数据

    int statu;          //tcpclient的状态 由EPOLLNODE决定
                        //0代表 正在从文件描述符中读取数据
                        //1代表 正在将sockbuf中的内容写入到文件描述符中去
                        //2代表 当前的状态处于空闲状态

                        //此状态我想由Node去决定
    enum Status{reading,writing,free,cannotus};
    AssistInfo(bool cur_fd,bool cur_cli,int fd_len_,int status_){
        read_cur=cur_fd;
        write_cur=cur_cli;

        fd_len=fd_len_;
        statu=status_;
        can_read=true;
    }
    AssistInfo(bool cur_,int fd_len_,int status_,bool canrw){
        read_cur=cur_;
        fd_len=fd_len_;
        statu=status_;
        can_read=canrw;
    }
    AssistInfo(){
        read_cur=true;
        fd_len=0;
        statu=AssistInfo::free;
        can_read=true;
    }
    void initAss(){
        read_cur=InMemPool;
        write_cur=InMemPool;
        fd_len=0;
        statu=AssistInfo::free;
        can_read=true;
        can_write=true;
    }
    void destory(){
        memset(this,0,sizeof(AssistInfo));
        statu=AssistInfo::cannotus;
    }
};

class Sock_Buff;

class TcpClient
{
public:
    friend class EpollNode;
    TcpClient();
    TcpClient(string ip,int prot,int cfd);

    TcpClient(const TcpClient &&cli);
    ~TcpClient();
    int read_cli(char *sock_buf,int len);
    int write_cli(char *sock_buf,int len);
    int nio_read();
    int nio_write();

public:
    //作为测试 暂时将该内容示为public:
    void initInfo(string ip,int prot,int cfd);//初始化ip 端口 cfd
    void setInfoAllLen(int len);              //设置这次发送信息的长度
                                              //并且初始化sockbuf中是否需要从sockbuf中获取内容 以及获取多少
    void initSockBuf(int size);
    void initSockBufMemPool(int size);        //初始化sockbuf中内存池的部分
    void setAssistInfo(bool cur_,int fd_len,int status);
    void destory();
    bool judeRWOver();
    bool judeCurOver();
    void doRWOver();
    void setAssisStatu(int statu);
    int  getFreeSize();                      //获取在内存池申请的内存空间以及在超大内存空间申请的内存大小
public:
    void getFromBigBuf();                   //从bigbuf中获取内存
    void sendBackBigBuf();                  //将bugbuf中的内容归还给bigbuf
    int  read_head();
    int  read_head(char *arry,int len);
private:
    string ip_;
    int prot_;
    int cfd_;
    Sock_Buff *sock_buf;
    AssistInfo assist;                      //记录sockbuf的状态 比如说处于从文件描述符中读取数据的状态
                                            //                                     写入的状态 还有空闲的状态
};

#endif // TCP_CLIENT_H
