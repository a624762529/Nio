#include "heartalarm.h"

HeartAlarm::HeartAlarm()
{
    m_canuse=false;
}

void HeartAlarm::startAlarm()
{
//    if(m_canuse)
//    {
//        cout<<"开始心跳循环"<<endl;
//    }
    while(m_canuse)
    {
        int ret=write(m_pipe[1],"a",1);
        if(ret==-1)
        {
            perror("pipe wirte error");
        }
        sleep(m_peralarm);
    }

}

void HeartAlarm::initAlarm(int time)
{
    //创建pipe
    if(pipe(m_pipe) == -1)
    {
        fprintf(stderr,"Can't create the pipe!");
        exit(2);
    }
    m_peralarm=time;
    m_canuse=true;
}


void HeartAlarm::addNode(TcpCommunication *com)
{
    if(com==nullptr)
    {
        return;
    }

    //如果节点在心跳队列上  那就自加相应的arg
    if(com->m_in_heart_que==true)
    {
        com->m_tag++;
        //cout<<"com__tag  value:"<<com->m_tag<<endl;
        return;
    }

    //不在心跳队列上 那就讲通讯对象加上
    com->m_tag=0;
    com->m_in_heart_que=true;
    m_vec[0].push_back(com);
}

vector<TcpCommunication *> HeartAlarm::timeout_Act
            (map<TcpCommunication*,bool>& mp)
{
    char buf[102]{0};
    int ret=read(m_pipe[0],buf,102);
    if(ret==-1)
    {
        perror("pipe read error");
    }
    vector<TcpCommunication *> vec=m_vec[3];
    m_vec[3].clear();
    moveVec();
    for(vector<TcpCommunication *>::iterator it=vec.begin();it!=vec.end();)
    {
        if(mp.find(*it)==mp.end())
        {
            vec.erase(it);
            continue;
        }

        if((*it)->m_tag==0)
        {
              it++;
        }
        else
        {
            m_vec[0].push_back(*it);
            (*it)->m_tag=0;
            vec.erase(it);
        }

    }
    return vec;    //反馈回去要清除的内容
}

void HeartAlarm::moveVec()
{
    for(int i=3;i>0;i--)
    {
        swap(m_vec[i],m_vec[i-1]);
    }
}

void HeartAlarm::start()
{
    thread th(&HeartAlarm::startAlarm,this);
    th.detach();
}

HeartAlarm::~HeartAlarm()
{
    m_canuse=false;
    close(m_pipe[0]);
    close(m_pipe[1]);
}

int HeartAlarm::getReadFd()
{
    return m_pipe[0];
}

int HeartAlarm::getWriteFd()
{
    return m_pipe[1];
}
