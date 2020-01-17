#ifndef MEM_POLL_H
#define MEM_POLL_H
#include"head.h"
#include<iostream>
using namespace std;
class Link;
class Mem_Poll;
class Node
{
public:
    friend class Link;
    Node();
    void add_Node(void *mem);
    void *get_Node();
private:
    Node *next;
};

class Link
{
public:
    friend class Mem_Poll;
    Link();
    int now_capsize();
    void add_Node_To_Link(void *mem);
    void *get_Node_From_Link();
    void clear_Mem();
private:
    int node_size;
    Node *head;
};

class Mem_Poll
{
public:
    Mem_Poll();
    ~Mem_Poll();

    //设置其移动拷贝构造函数 还有赋值函数
    //移动拷贝构造函数 移动赋值函数禁用
    Mem_Poll(const Mem_Poll&)=delete;
    Mem_Poll(const Mem_Poll&&)=delete;
    void operator=(const Mem_Poll&)=delete;
    void operator=(const Mem_Poll&&)=delete;
public:
    //公共接口
    //从池子中得到某个槽中的数据
    void* get_Mem_From_Slot(int size);
    void put_Mem_To_Slot(const void *mem,int size);
private:
    //关于二级内存池操作的函数
    bool fillSecPoll();
    void *getMemFromSecPoll(int size);//从二级内存池中获取相应的内存
    void addSecMemToSlot();           //二级内存池中的数据不够分配给需要的内存
                                      //将至分配到二级内存槽中

    void ifSecMemNotEnough(int size); //如果二级内存池中的数据不够　那就做对应的操作
private:
    //针对三个槽区操作的区域
    void fillAllSlot();
    void slotReFill(int i);
    void zeroSlot();
    void putMemToSlot(const void *mem,int size);
    void *getMemFromSlot(int size);
    void *getMemFromNextSlot(int index);
private:
    //一级内存池中 维护3种节点 1024 2048 4096 这三种内存块
    Link poll[3];
    //二级内存池
    //要是一级内存池大小不足 那就使用二级内存池做分配
    void *mem_sec;
    int   now_size;
    //内存池的垃圾回收机制
    Link trash;
};


//使用单例对它进行封装

class Sig_Pool
{
public:
    Sig_Pool();
    ~Sig_Pool();
    void *alloc_Mem(int size);
    void put_Mem(int size,void *mem);
private:
    mutex mu;
    static Mem_Poll *mem_pool;
};


extern Sig_Pool alloc;
#endif // MEM_POLL_H
