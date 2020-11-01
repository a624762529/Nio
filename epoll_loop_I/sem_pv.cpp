#include "sem_pv.h"

Sem_Pv::Sem_Pv()
{
    this->magic=0;
    this->free_qua=1;
}
Sem_Pv::Sem_Pv(int val)
{
    this->magic=0;
    this->free_qua=val;
}
Sem_Pv::~Sem_Pv()
{
    this->magic=0;
    this->free_qua=0;
    this->waiting_num=0;
}
void Sem_Pv::set_CanUse()
{
    this->magic=MagicNum;
}
//V操作 取出元素
void Sem_Pv::sem_V()
{
    if(magic!=MagicNum)
    {
        cout<<"使用了未经启用的信号量"<<endl;
        throw bad_alloc();
    }
    again:
    unique_lock<mutex> lock(mu);
    while (free_qua<=0)
    {
        waiting_num++;
        con.wait(lock,[this](){return free_qua>0;});
        waiting_num--;
    }
    {
        lock_guard<mutex> lock(mx);

        int ret=free_qua;
        free_qua--;
        if(ret>0)
        {
            return ;
        }
        else
        {
            free_qua++;
            goto again;
        }
    }
}
//P操作 往其中存入元素
void Sem_Pv::sem_P()
{
    if(magic!=MagicNum)
    {
        cout<<"使用了未经启用的信号量"<<endl;
        throw bad_alloc();
    }
    unique_lock<mutex> lock(mu);
    free_qua++;
    if(waiting_num==0)
    {
        return;
    }
    else
    {
        con.notify_one();
        return;
    }
    //con.notify_one();
}

void Sem_Pv::set_Val(int val)
{
    lock_guard<mutex> lock(mu);
    free_qua=val;
    return ;
}
void Sem_Pv::sem_Try_P()
{
    return sem_P();
}
bool Sem_Pv::sem_Try_V()
{
    if(magic!=MagicNum)
    {
        cout<<"使用了未经启用的信号量"<<endl;
        throw bad_alloc();
    }
    again:
    unique_lock<mutex> lock(mu);
    while (free_qua<=0)
    {
       return false;
    }


    {
        lock_guard<mutex> lock(mx);

        int ret=free_qua;
        free_qua--;
        if(ret>0)
        {
            return true;
        }
        else
        {
            free_qua++;
            return false;
        }
    }
    return false;
}
