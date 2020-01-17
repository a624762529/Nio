#include "threadpool.h"

ThreadPool::ThreadPool()
{

}


void ThreadPool::spy()
{
    //监控
}

//取出数据

shared_ptr<TaskImpl> ThreadPool::ThreadPool::Consumer()
{
    //从任务队列中取出一个之后返回
    lock_guard<mutex> lock(m_vecmutex);
    if(m_vectask.size())
    {
        auto ret=*m_vectask.begin();
        m_vectask.erase(m_vectask.begin());
        return ret;
    }
    else
    {
        return nullptr;
    }
}

void ThreadPool::Produce(shared_ptr<TaskImpl> val)
{
    //将val压入其中
    {
        lock_guard<mutex> lock(m_vecmutex);
        m_vectask.push_back(val);
    }
    m_task.sem_V();
}

shared_ptr<TaskImpl> ThreadPool::get()
{
    m_task.sem_V();
    auto task=Consumer();

    return task;
}


