#ifndef SOCK_BUFF_H
#define SOCK_BUFF_H

#include"head.h"
#include"tcp_client.h"
#include"tcp_buf.h"
#include"mem_poll.h"
#include<iostream>
#include<functional>
using namespace std;
/*
    关于Sock_Buf的设计思路
        在非阻塞网络中 要是对端 网路特别差 那就会写入失败
        那就将对应的文件描述符挂到 epoll树的写的集合
        {
            此时有两种状况
                状况1： 对端在心跳未超时 网路良好 那就万事大吉 恢复通讯即可
                状况2： 对端心跳超时    从epoll树拿下了该节点 阻断通讯
                       至于剩余未发送的信息作何处理 那是使用者该考虑的事情
        }
*/
/*
    关于Big_Sock_Buf的设计思路
        如果对端发送的数据超过了40K 那就用Big_Sock_Buf进行收发数据

        管理空闲块的思路类似于操作系统中的空闲块的管理(首次适应算法)
                                                12.6日 我在这个基础之上做了些许改进 、
                                                      为了保留大块的内存 寻找空闲内存块的时候 找相对少的空闲内存碎片
*/
/*
    12.5日9:20更新BigSockBuf
       针对于Sock_Buf中具体可调用多少BigSockBuf网络资源：思路如下
            如果发送的信息在30K一下  那就用自身的资源起解决
            如果发送的信息处于 30K-100K 可尝试调用10×1024的资源量
            如果发送的信息在  100K-400K 之间 可尝试调用30*1024 的资源量
            超过400K 那就把它扔到线程池里去
            因为我对北极星的期望是用它解决短连接 并且发送的数据不是特别庞大的状况
                                                    （即内存小于400*1024的信息交互）

       函数日志

            在归还节点的时候 要是要归还区间中的首部==对方的尾部
                                         尾部==对方的首部
                                         那就将它们归并
            loop即可
            loop结束的标志 -- 要是下一个节点得不到满足的情况，那就结束它
                             因为空闲索引在 vec中处于从大到小的状况

            申请节点 参数信息就是要申请的节点大小的信息
                    要是能刚好找到合适的节点 那就直接申请好了
                    要是没有足够大的 那就给出现在bigbuff中最大的空闲区间
                    要是 所有的节点都过大了 那就找到最小节点 从中划出需要的部分
*/

class Big_Sock_Buf
{
public:
    friend class Sock_Buff;
    class Free_Node
    {
    public:
        int start;
        int end;
        int size;
    public:
        Free_Node()
        {
            start=0;
            end=0;;
            size =end-start;
        }
        Free_Node(int st,int ed)
        {
            start =st;
            end=ed;
            size =end-start;
        }
        Free_Node breaku(int free_size)
        {
            if(free_size<this->get_free_size())
            {
                //当前的区块足矣满足
                int ed=this->start+free_size;
                Free_Node ret(this->start,ed);
                this->start=ed;
                update_size();
                return ret;
            }
            //总觉得这一步是一种浪费 不过强迫症使然 加上了这最后一句话
            return Free_Node(0,0);
        }
        bool Merge(Free_Node mer)
        {
            //归并的思路
            //        如下
            if(mer.start==this->end)
            {
                this->end=mer.end;
                update_size();
                return true;

            }
            else if(mer.end==this->start)
            {
                this->start=mer.start;
                update_size();
                return true;

            }
            return false;
        }
        int get_free_size()
        {
            return size;
        }
        void update_size()
        {
            size=end-start;
        }
    };
public:
    class Free_Node;
    Big_Sock_Buf();
    pair<Big_Sock_Buf::Free_Node,bool> try_Get_Mem(int statu);
    bool put_Mem(int begin,int end);
    char *getHandler();
    //归还的思路:
private:
    mutex mu;
    char  big_arry[64*1024];
    vector<Free_Node> free_box;
};



/*
    SockBuf 运行流程 ：
        使用tcp传送文件的时候 先发送一个int（长度）
        利用int去初始化节点元素
*/
class Sock_Buff
{
public:
    //friend class TcpSer;
    friend class TcpClient;
    Sock_Buff();
    ~Sock_Buff();
public:
    void initSockBuf(int alloc_from_mem_pool);                 //第一个参数的意思是从内存池申请的内存长度
    void setInfoAllLen(int infolen);                           //设置 文件的的长度以及决策是否从文件描述符
    int  getInfoLen();                                         //获取文件的长度
public:
    //针对内存池内容 的操作
    int getFreeSizeMemPool();                                  //获取从内存池申请的空闲内存块
    int getFreeSizeBigBuf();                                   //获取超大内存块的长度

    int getNoFreeSizeMemPool();                                //获取从内存池中申请的空间的非空闲内存大小
    int getNoFreeSizeBigBuff();                                //超大内存空间块的总长度
    int getNofreeSize();

    int getSizeFromMemPool();                                  //获取从内存池申请的空间大小

    inline int getLenFromMemPool();

    void getFreeFromMemPool(char *&buf,int &len);
    void getNoFreeFromMemPool(char *&buf,int &len);
    void moveEndBackMemPool(int len);                  //移动尾部节点的属性
    void moveStartBackMemPool(int len);

public:
    //针对超大内存块的操作
    int getMemFromBigBuf();                                    //获取从超大内存上申请空间的大小
    //清理sockbuf中的信息 由于buf中的内容是tcpclient
    //所以buf中的内容由tcpclient去归还给内存池子
    void destorySockBuf();
    void sendOver();
                                                                //一次通讯结束(归还bigbuf内存以及清理文件长度)
    void sendBackMemToBigMem();
    void getFreeFromBigBuf(char *&buf,int &len);
    void getNoFreeFromBigBuf(char *&buf,int &len);
    void moveEndBackBigBuf(int len);
    void moveStartBackBigBuf(int len);
    bool get_need_big_buf();
    bool get_in_big_buff();

private:
    inline int getStata(int info_len);
    void judeGetBufFromBuf(int info_len);                      //决策是否需要从超大内存池中申请节点
public:
    class Info
    {
    public:
        int  start_index;      //超大索引块开始的索引
        int  end_index;        //超大索引块结束的索引
        int  alloc_len;        //实际申请的总长度
        int  no_free_begin_index; //非空闲内存开始的地方
        int  no_free_end_index;   //非空闲内存结束的地方
    };
private:
    /*
        从内存池中申请的内存的范围是[0,size);
        所以仅记录非空闲块的范围即可 其范围表示方式仍用左闭 右开表示
        填充逻辑 先填充从内存池中申请的空闲块的内存
                再填充从bigbuf中申请的内存块  从前到后的填充
    */

    /*
        因而在访问空闲内存区间的时候：
            内存块的总数-非空闲块的end索引   此值是空闲索引开始的索引
    */

    int info_length;                //信息的总长度
    int alloc_from_mempool;         //信道的总长度 即从内存池中申请的内存总长度
    int buf_len;                    //buf的总长度
    int begin_nofree_index;         //非空闲索引 开始的地方 内存池中申请的节点信息
    int end_nofree_index;           //非空闲索引结束的地方
    char *buf;                      //实际存放内容的区域


    int  needallocfrombigbuf;       //逻辑上需要从bigbuf中申请多少内存片
    bool need_big_buf;              //是否需要从超大内存中申请内存碎片
    bool in_big_buff;               //是否从超大Sock中申请到了元素
    Info bigSockInfo;               //info信息我想在读写的时候 初始化它

};

extern Big_Sock_Buf myBigBuf;
#endif // SOCK_BUFF_H
