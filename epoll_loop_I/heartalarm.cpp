#include "heartalarm.h"

HeartAlarm::HeartAlarm()
{
    this->alarm=60;
    magic=0;
}

HeartAlarm::HeartAlarm(int per_time)
{
    this->alarm=per_time;
    magic=0;
}

void HeartAlarm::initHeartAlarm()
{
    int ret=pipe(pid_fd);
    if(ret==-1)
    {
        string err_info=to_string(__LINE__)+"create pipe err";
        perror(err_info.c_str());
        throw bad_alloc();
    }
    //将两者设置为非阻塞
    int old_option=fcntl(pid_fd[0],F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(pid_fd[0],F_SETFL,new_option);

    old_option=fcntl(pid_fd[1],F_GETFL);
    new_option=old_option|O_NONBLOCK;
    fcntl(pid_fd[1],F_SETFL,new_option);

    //0是读端　１是写段
    thread th=std::move(thread(&HeartAlarm::startAlarm,this));
    th.detach();
}

void HeartAlarm::startAlarm()
{
    int write_fd=pid_fd[1];
    //当magic不为0的时候
    cout<<"start heart alarm"<<endl;
    while (magic==magic_num)
    {
        //每经过alarm秒　就发送数据　让读端口可读
        sleep(alarm);
        char ch='0';
        int ret=write(write_fd,&ch,1);
        if(ret==-1)
        {
            perror("write err");
        }
    }
    cout<<"end heart alarm"<<endl;
}

int HeartAlarm::getAlarmFd()
{
    //返回读端
    return pid_fd[0];
}


void HeartAlarm::moveNext()
{
    //释放掉尾端元素
    for(auto node:vec_charge[3])
    {
        if(node!=nullptr)
            node->judeChatOver();
    }
    //  3 2   2 1  1 0
    for(int i=3;i>0;i--)
    {
        swap(vec_charge[i],vec_charge[i-1]);
    }
}

void HeartAlarm::updateIndex(shared_ptr<EpollNode> node)
{
    vec_charge[0].push_back(node);
}

void HeartAlarm::setPerHeart(int time)
{
    alarm=time;
}


void HeartAlarm::setCanUse()
{
    this->magic=magic_num;
    initHeartAlarm();
}

void HeartAlarm::stopHeart()
{
    this->magic=0;
}

void HeartAlarm::readChar()
{
    char ch='/0';
    int ret=read(pid_fd[0],&ch,1);
    if(ret==-1)
    {
        if(errno==EWOULDBLOCK)
        {
            perror("nio read err:");
        }
        else
        {
            perror("pipe read err:");
            throw bad_alloc();
        }
    }
    cout<<"read char:"<<ch<<endl;
}
