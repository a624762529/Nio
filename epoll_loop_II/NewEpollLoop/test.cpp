#include"tcplisten.h"
#include"tcpcommunication.h"
#include<iostream>
#include"epollloop.h"
using namespace std;
void readCallBack(char* arry,int len,void *arg,TcpCommunication *tcpCon)
{    
    if(len>0)
    {
        tcpCon->setInfoLen(len);
        tcpCon->addInfoToWrite(arry,len);
        tcpCon->changeStatus(TcpCommunication::Status::Write);
    }
    else if(len==0)
    {
        cout<<"断开链接"<<__FUNCTION__<<__LINE__<<endl;
        //清理arg
    }
}

void writeCallBack(void *arg,TcpCommunication *tcpCon)
{
    //cout<<"写入的信息长度:"<<len<<endl;
    tcpCon->changeStatus(TcpCommunication::Status::Read);
}

void acceptCallBack(TcpCommunication *tcpCon)
{
    //cout<<"accept callback"<<endl;
    //通过这个函数给相应的accept加参数
}

//测试客户端 和服务端的连,void *read_arg接
int main02(int argc, char *argv[])
{
    TcpListen listen(8888);
    pair<int,sockaddr_in> ret;
    while (true)
    {
        ret=listen.acceptConnect();
        if(ret.first==-1)
        {
            sleep(1);
        }
        else
        {
           TcpCommunication commun(ret.first,ret.second);
           commun.setReadCallBack(readCallBack,(void *)&commun);
           commun.setWriteCallBack(writeCallBack,(void *)&commun);
           commun.changeStatus(TcpCommunication::Status::Read);
           while (true)
           {
               commun.callReadCallBack();
               commun.callWriteCallBack();
           }

        }
    }
    return 1;
}


int main()
{
    EpollLoop loop(8888);

    loop.setReadcb  (readCallBack);
    loop.setWritecb (writeCallBack);
    loop.setAcceptcb(acceptCallBack);
    loop.setCanUse();

    loop.initHeartTime(2);
    loop.startAlarm();
    loop.loop();
    return 1;
}
