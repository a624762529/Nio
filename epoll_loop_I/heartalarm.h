#ifndef HEARTALARM_H
#define HEARTALARM_H
#include"head.h"
#include"epollnode.h"

#define magic_num 11
//使用管道操作进行通讯 没经过60秒发送一次超时信息  3*60秒没有心跳发送过来　那就挂掉
class EpollNode;

class HeartAlarm
{
public:
    friend class Epoll;
    HeartAlarm();
    HeartAlarm(int per_time);
    void setPerHeart(int time);
    void moveNext();                                //信号触发　导致桶节点前移 并且　清除掉最后一段
    void updateIndex(shared_ptr<EpollNode> node);   //直接将该节点插入到首部就行了
    int  getAlarmFd();
    void stopHeart();
    void readChar();                                //从读端读取一个字符
private:
    void setCanUse();
    void startAlarm();

private:
    int magic;
    void initHeartAlarm();
    int alarm;
    int pid_fd[2];
    vector<shared_ptr<EpollNode> > vec_charge[4];
};

#endif // HEARTALARM_H
