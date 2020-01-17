#ifndef SEM_PV_H
#define SEM_PV_H
#include"head.h"
#include<atomic>
using namespace std;

/*
    既然我已经在算计 把将北极星系统写成多线程
    陈硕先生说使用条件变量宛如铅笔刀在削大树
    既然如此 我索性自己写一套pv锁
    来节省今后的精力 孙然蛮想用Linux自带的api 不过 北极星  系统的实现基础毕竟是C++
    我想着尽我所能的用C++ 哪怕将来再度更新这个库的时候 使用宏去设置它的linux api
    我就用C++11造轮子把
*/

/*
    实现原理：
        使用生产者 与消费者实现
    信号量的初值一经设定不容改变
*/
#define MagicNum 11
//当magic为11的时候方可启动信号量
class Sem_Pv
{
public:
    Sem_Pv();
    Sem_Pv(int val);
    ~Sem_Pv();
    void set_CanUse();
    void sem_P();      //put 在信号量中存放元素
    void sem_Try_P();  //非阻塞往信号量中存入元素
    void sem_V();      //get 在信号量中取出元素
    bool sem_Try_V();  //非阻塞往信号量中取出元素
    void set_Val(int val);
private:
    int magic;                    //设置信号量是否可用
    atomic<int> waiting_num;      //等待的线程数目

    atomic<int> free_qua; //当前的信号量中 有几个可用资源
    mutex mu;
    condition_variable con;

    mutex mx;
};

#endif // SEM_PV_H
