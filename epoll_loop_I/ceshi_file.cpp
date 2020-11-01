#pragma once
#include"head.h"

#include"mem_poll.h"
#include"sem_pv.h"
#include"sock_buff.h"
#include"logperf.h"
#include"tcp_ser.h"
#include"tcp_client.h"
#include"epollloopi.h"
#include"epollnode.h"


#include<vector>
using namespace std;

//测试link是否可用
int main(int argc, char *argv[])
{
    //测试内存池中link类是否可用做到 加入节点 获取节点
    double arry[10];
    memset(arry,0,sizeof(arry));

    Link node;
    for(int i=0;i<5;i++)
    {
        node.add_Node_To_Link(reinterpret_cast<void*>(&arry[i*2]));
    }
    for(int i=0;i<5;i++)
    {
        int *p= reinterpret_cast<int*>(node.get_Node_From_Link());
        *p=100;
    }
    return 1;
}

////测试内存池的功能
//int main00(int argc, char *argv[])
//{
//    //测试其构造函数
//    Mem_Poll poll;
//    //申请30个索引为1的节点
//    poll.~Mem_Poll();
//    return 1;
//}
//int main01()
//{
//    //测试其构造函数
//    Mem_Poll poll;

//    //测试一级内存槽中的数据使用结束 获取二级内存池
//    //的数据内容
//    for(int i=0;i<40;i++)
//    {
//        char *p=reinterpret_cast<char*>
//                (poll.get_Mem_From_Slot(1024));
//        for(int i=0;i<1024;i++)
//        {
//            p[i]='1';
//        }
//    }

//    //测试二级内存池的内容耗尽

//    //至于测试内存耗尽的问题 只能手动测试了---》无奈 假设它申请成功
//    //然后 设置其为0 可行
//    for(int i=0;i<400;i++)
//    {
//        char *p=reinterpret_cast<char*>
//                (poll.get_Mem_From_Slot(2048));
//        for(int i=0;i<2048;i++)
//        {
//            p[i]='1';
//        }
//    }

//    //至于测试内存耗尽的问题 只能手动测试了---》无奈 假设它申请成功
//    //然后 设置其为0

//    for(int i=0;i<10;i++)
//    {
//        char *p=reinterpret_cast<char*>
//                (poll.get_Mem_From_Slot(1024*3));
//        for(int i=0;i<1024*3;i++)
//        {
//            p[i]='1';
//        }
//    }

//    //申请30个索引为1的节点
//    poll.~Mem_Poll();
//    return 1;
//}
////12.1 内存池整体测试结束

//int a=0;
//void test_mutex(Sem_Pv &sem)
//{
//    for(int i=0;i<10000;i++)
//    {
//        sem.sem_V();
//        cout<<a<<endl;
//        a++;
//        sem.sem_P();
//    }
//}
////关于C++11实现的同步信号量的测试
//int main03(int argc, char *argv[])
//{
//    //默认情况下 sem类似于一把锁的功能
//    Sem_Pv sem;
//    sem.set_CanUse();
//    //测试它的互斥性
//    thread th[4];
//    for(int i=0;i<4;i++)
//    {
//        th[i]=std::move(thread(test_mutex,std::ref(sem)));
//    }
//    for(int i=0;i<4;i++)
//    {
//        th[i].join();
//    }
//    return 0;
//}


////测试其同步性 使用生产者与消费者进行同步测试
//class Test_async
//{
//public:
//    Test_async()
//    {
//        sem_empty.set_CanUse();
//        sem_not_empty.set_CanUse();
//        vec.clear();
//        sem_empty.set_Val(7);
//        sem_not_empty.set_Val(0);
//    }
//    ~Test_async()
//    {
//        vec.clear();
//        sem_empty.set_Val(0);
//        sem_not_empty.set_Val(0);
//    }
//    void programer()
//    {
//        //生产者线程 往vec中存放东西


//        sem_empty.sem_V();
//        mu.lock();
//          promer++;
//        //临界区
////        auto thread_id=this_thread::get_id();
////        int *p=(int *)(&thread_id);
////        static int i=0;
//        //cout<<"生产者者生产的数据为"<<i<<"   thread_id:"<<(*p)%1000<<endl;
//        //vec.insert(vec.begin(),i);
//        //i++;
//        mu.unlock();
//        sem_not_empty.sem_P();
//    }
//    void consumer()
//    {
//        //消费者线程 往vec中存放东西

//        sem_not_empty.sem_V();
//        mu.lock();
//         comer++;
//        //临界区
////        auto thread_id=this_thread::get_id();
////        int *p=(int *)(&thread_id);
////        //cout<<"消费者获取的数据为"<<vec.back()<<"   thread_id:"<<(*p)%1000<<endl;
////        vec.pop_back();

//        mu.unlock();
//        sem_empty.sem_P();
//    }
//private:
//    vector<int> vec;
//    Sem_Pv sem_empty;
//    Sem_Pv sem_not_empty;
//    mutex mu;
//public:
//    atomic<int> comer;
//    atomic<int> promer;
//};
//Test_async test;
//void thread_pro()
//{
//    for(int i=0;i<777;i++)
//    {
//        test.programer();
//        int ran=random()%2;
//        //sleep(ran);
//    }
//}
//void thread_com()
//{
//    for(int i=0;i<777;i++)
//    {
//        test.consumer();
//        int ran=random()%2;

//    }
//}
//int main04()
//{

//    //三 消费者 一个生产者  测试在多线程的条件下 消费者的速度大于生产者
//    //    thread th[4];
//    //    th[0]=std::move(thread(thread_com));
//    //    th[1]=std::move(thread(thread_com));
//    //    th[2]=std::move(thread(thread_pro));
//    //    th[3]=std::move(thread(thread_com));
//    //    for(int i=0;i<4;i++)
//    //    {
//    //        th[i].join();
//    //    }

//   // 三 生产者 一个消费者  测试在多线程的条件下 生产者生产的速度大于消费者
//    //        thread th[4];
//    //        th[0]=std::move(thread(thread_pro));
//    //        th[1]=std::move(thread(thread_pro));
//    //        th[2]=std::move(thread(thread_pro));
//    //        th[3]=std::move(thread(thread_com));
//    //        for(int i=0;i<4;i++)
//    //        {
//    //            th[i].join();
//    //        }

//    //关于信号量的丢包测试 不存在丢包现象
//    thread th[2];
//    th[0]=std::move(thread(thread_pro));
//    th[1]=std::move(thread(thread_com));
//    for(int i=0;i<2;i++)
//    {
//        th[i].join();
//    }
//    cout<<test.comer<<endl;
//    cout<<test.promer<<endl;
//    return 1;
//}

////针对信号量的边缘测试
////12.4 测试结果 如果信号量的初值为4 那它可以被V四次  初步测试 测试效果达到预期

//int main05()
//{
//    Sem_Pv sem(4);
//    sem.set_CanUse();

//    for(int i=0;i<5;i++)
//    {
//        sem.sem_V();
//        cout<<i<<endl;
//    }
//    return 1;
//}

////测试性能日志的写入功能
//int main06()
//{

//    string info="1111111111";
//    string if_;
//    for(int i=0;i<100;i++)
//    {
//        if_.append(info);
//    }
//    long now=clock();
//    Logperf log("./file_log",O_CREAT);
//    log.set_can_used();
//    log.start_write();
//    for(int i=0;i<1000000;i++)
//    {
//        log.Put_into_log(if_);
//    }
//    long over=clock();
//    cout<<"文件写入结束 花费的时间是"<<over-now<<endl;

//    cout<<"write 次数"<<log.record_write_file<<"  "<<endl;

//    return 1;
//}
////测试Sock_Buf的性能
//int main07()
//{
//    Big_Sock_Buf sock_buf;
//    vector<pair<Big_Sock_Buf::Free_Node,bool>> vec_;
//    //测试其申请资源的状况

//    //预测的测试结果: 前6个满足需求 倒数第二个 因为 空间中资源不够而将剩余内存给他
//                    //倒数第一个不满足需求
//    for(int i=0;i<8;i++)
//    {
//        pair<Big_Sock_Buf::Free_Node,bool> index=
//                           sock_buf.try_Get_Mem(1024*10);
//        vec_.push_back(index);
//    }
//    vec_.pop_back();
//    //归还测试
//    //测试方案 顺序归还测试结束 无bug
//    //

//    //    for(vector<pair<Big_Sock_Buf::Free_Node,bool>>::iterator it=vec_.begin();
//    //        it!=vec_.end();it++)
//    //    {
//    //        sock_buf.put_Mem((*it).first.start,(*it).first.end);
//    //    }


//    //非顺序归还测试方案
//    //    while (vec_.size()!=0)
//    //    {
//    //        int size=vec_.size();
//    //        int rd=rand()%size;

//    //        vector<pair<Big_Sock_Buf::Free_Node,bool>>::iterator
//    //                it=vec_.begin()+rd;
//    //        pair<Big_Sock_Buf::Free_Node,bool> act=*it;
//    //        vec_.erase(it);
//    //        sock_buf.put_Mem(act.first.start,act.first.end);
//    //    }

//    cout<<"归还结束"<<endl;
//    return 1;
//}

//int main08()
//{
//    Sock_Buff *sock=nullptr;
//    sock=reinterpret_cast<Sock_Buff*>(alloc.alloc_Mem(1024*3));
//    sock->initSockBuf(1024*3);
//    sock->destorySockBuf();
//    //alloc.put_Mem(sock->getLenFromMemPool(),sock);
//    return 1;
//}
////测试tcpser的connect功能
//int main09()
//{
//    //针对于通讯文件描述符的accept测试成功
//    TcpSer ser(8888);
//    ser.init_Ser();
//    pair<int,sockaddr_in> ret=ser.nio_accept();
//    cout<<"文件描述符为"<<ret.first<<endl;
//    char ip[64];
//    memset(ip,0,sizeof(ip));
//    printf("new connect [%s:%d],pos[%d]\n",
//              inet_ntoa(ret.second.sin_addr), ntohs(ret.second.sin_port));
//    //因暂时没有想好内存池的选择功能 所以我只能。。。给每个tcpclient分配1024*3的内存碎片
//    //针对于客户端的读写进行测试
//    return 1;
//}

//int main10()
//{
//    cout<<sizeof(TcpClient)<<endl;


//    int fd=open("./testfile",O_RDWR|O_CREAT,777);
//    int wfd=open("./wrfile",O_RDWR|O_CREAT, 777);

////    //测试读部分
//    {
////        TcpClient cli("127.0.0.1",8888,fd);

////        cli.initSockBufMemPool(1024*3);
////        //设置传送信息的长度
////        cli.setInfoAllLen(1024*70);
////        cli.doRWOver();
////        //从超大内存中获取长度

////        cli.setInfoAllLen(1024*70);

////        cli.nio_read();
////        cli.nio_read();
////        cli.nio_read();
////        //测试clientwrite
//////        while (true)
//////        {
//////            char buf[2048*10];
//////            int ret=cli.read_cli(buf,sizeof(buf));
//////            cli.doRWOver();
//////            cout<<ret<<endl;
//////        }

////        char buf[2048*10];
////        int ret=0;
////        while (true)
////        {
////            ret=cli.read_cli(buf,sizeof(buf));
////            cout<<ret<<endl;
////            cli.nio_read();
////        }



////        cli.nio_read();
////        cli.nio_read();
////        ret=cli.read_cli(buf,sizeof(buf));
////        cout<<ret<<endl;

////        //测试重新启用它 判断它是否可用
////        cli.destory();
//    }


//    //测试写部分
//    {
//        TcpClient cli("127.0.0.1",8888,wfd);
//        cli.initSockBufMemPool(1024*3);
//        //设置传送信息的长度
//        cli.setInfoAllLen(1024*40);
//        cli.doRWOver();

//        cli.setInfoAllLen(1024*70);
//        string info="11111 1111";
//        string wriinfo;
//        info.append(1,'\n');
//        for(int i=0;i<600;i++)
//        {
//            wriinfo.append(info);
//        }

//        while (true)
//        {
//            for(int i=0;i<20;i++)
//            {
//                int ret=cli.write_cli
//                    (const_cast<char*>(wriinfo.c_str()),wriinfo.size());

//               if(ret==0)
//               {
//                   break;
//               }
//            }
//            int ret=cli.nio_write();
//            if(ret==0)
//            {
//                break;
//            }
//            cout<<ret<<endl;
//        }



//        for(int i=0;i<20;i++)
//        {
//            int ret=cli.write_cli
//                (const_cast<char*>(wriinfo.c_str()),wriinfo.size());
//            if(ret==0)
//            {
//                break;
//            }
//        }
//        cli.nio_write();


//        //cli.sendBackBigBuf();
//        cli.destory();
//    }
//    return 1;
//}

void fun_read(EpollNode *cli,void*)
{
    char buf[4096];
    while (true)
    {

        int ret=cli->read_cli(buf,sizeof(buf));
        if(ret==0)
        {
            break;
        }
        cout<<ret<<endl;
    }
}


int main11()
{

    //测试node的初始化
    int fd=open("./testfile",O_RDWR|O_CREAT,777);
    int wfd=open("./wrfile",O_RDWR|O_CREAT, 777);
    EpollNode node("127.0.0.1",8888,fd);
    node.setInfoLen(4096*100);
    node.setCanUse();
    node.judeChatOver();


    node.setReadInfo(fun_read,NULL);
    //黏包测试
    node.callReadBack();
    node.callReadBack();



    node.callReadBack();
    node.callReadBack();
    node.callReadBack();
    node.callReadBack();

    //node.judeChatOver();

    //作为启动节点
    node.setInfoLen(4096*4);
    //设置其可以被使用
    node.setCanUse();


    node.callReadBack();
    node.callReadBack();
    node.callReadBack();
    node.callReadBack();
    node.callReadBack();
    node.callReadBack();


    //测试epoll的callback功能
    close(fd);
    close(wfd);
    return 1;
}

////测试epollnode节点的读回调　以及报头的接受　以及文本的发送
//int main12()
//{
//    //创建监听信息
//    TcpSer ser(8888);
//    ser.init_Ser();
//    //accept
//    pair<int,sockaddr_in> ret=ser.nio_accept();
//    cout<<"文件描述符为"<<ret.first<<endl;
//    char ip[64];
//    memset(ip,0,sizeof(ip));
//    printf("new connect [%s:%d],pos[%d]\n",
//              inet_ntoa(ret.second.sin_addr), ntohs(ret.second.sin_port));
//    //初始化通讯节点
//    EpollNode node(inet_ntoa(ret.second.sin_addr),
//                   ntohs(ret.second.sin_port),ret.first);
//    //设置callbackinfo
//    node.setReadInfo(fun_read,nullptr);
//    //callback　测试读回调

//    node.callReadBack();
//    node.callReadBack();
//    node.callReadBack();
//    node.callReadBack();

//    //测试node节点移动之后的
//    return 1;
//}


void funTestRead(EpollNode *cli,void*)
{
    char buf[4096];
    cout<<"可读"<<endl;
//    while (true)
//    {

//        int ret=cli->read_cli(buf,sizeof(buf));
//        if(ret==0)
//        {
//            break;
//        }
//        cout<<ret<<endl;
//    }
    cli->changeNodeEvent(EPOLLOUT);

}

//测试Epoll的并发功能
//int main999()
//{
//    //测试epoll的事件循环机制
//    TcpSer ser(8888);
//    ser.init_Ser();
//    Epoll ep_loop;
//    ep_loop.setListenSock(ser);
//    ep_loop.loop();
//    return 1;
//}


//void funTestWrite(EpollNode *cli,void*)
//{
//    cout<<"可写"<<endl;
//    cli->changeNodeEvent(EPOLLIN);
//}
//int main122()
//{
//    //    TcpSer ser(8888);
//    //    ser.init_Ser();
//    shared_ptr<TcpSer> ser_pt(new TcpSer(8888));
//    ser_pt->init_Ser();

//    ser_pt->setReadcb(funTestRead,NULL);
//    Epoll ep_loop;
//    ep_loop.setListenSock(ser_pt);
//    ep_loop.loop();
//    return 1;
//}
//int main1233()
//{
//    EpollLoop lp;
//    shared_ptr<TcpSer> ser_pt(new TcpSer(8888));
//    ser_pt->init_Ser();
//    ser_pt->setReadcb(funTestRead,NULL);
//    ser_pt->setWritcb(funTestWrite,NULL);

//    lp.setAlarm(4);

//    lp.addSer(ser_pt);
//    lp.loop();
//    return 1;
//}
