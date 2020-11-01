#ifndef HEARTALARM_H
#define HEARTALARM_H
#include"head.h"
#include"tcpcommunication.h"
#include<vector>
#include<map>

using namespace std;
class HeartAlarm
{
public:
    HeartAlarm();
    ~HeartAlarm();
    void initAlarm(int time);
    void startAlarm();
    void addNode(TcpCommunication *com);
    void start();

    int getReadFd();
    int getWriteFd();
    vector<TcpCommunication *> timeout_Act
                (map<TcpCommunication *, bool> &mp);
private:
    void                       moveVec();
private:
    int m_pipe[2];
    int m_peralarm;
    bool m_canuse;
    vector<TcpCommunication *> m_vec[4];
};

#endif // HEARTALARM_H
