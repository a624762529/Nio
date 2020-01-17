#ifndef THREADPOOL_H
#define THREADPOOL_H
#include"head.h"
#include"sem_pv.h"
#include<iostream>
using namespace std;

//模板T　必须实现一个doit的接口 在doit中实现自己的方法
class TaskImpl
{
public:
    virtual void doit()=0;
};

class ThreadPool
{
//使用pv锁　无法销毁线程 头疼
public:
    ThreadPool();
    shared_ptr<TaskImpl> get();
    void put(shared_ptr<TaskImpl> cont);
private:
    void spy();
    shared_ptr<TaskImpl> Consumer();
    void Produce(shared_ptr<TaskImpl> val);
private:
    Sem_Pv m_task;              //任务队列中任务 限制线程进入
    atomic<int> m_workingthread;//正在工作的线程
    mutex     m_vecmutex;       //维持任务队列的互斥访问
    vector<shared_ptr<TaskImpl> > m_vectask;   //任务队列
    atomic<int> waiting_num;//正在等待的线程数目

    mutex m_thread;             //线程锁  他和下面的条件变量的作用是为了控制线程的工作　以及死亡(让线程自动结束自己的生命)
    //conditional m_thcon;
    atomic<int> m_working;      //正在工作的线程的数目与
    atomic<int> m_sleeping;     //正在休息的线程的数目
};

#endif // THREADPOOL_H
