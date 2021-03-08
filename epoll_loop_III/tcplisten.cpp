#include "tcplisten.h"

TcpListen::TcpListen(int port)
{
    m_magic=CanUse;
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    // 创建套接字
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化服务器 sockaddr_in
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;                   // 地址族
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 监听本机所有的IP
    serv_addr.sin_port = htons(port);                 // 设置端口
    // 绑定IP和端口
    bind(lfd, (struct sockaddr*)&serv_addr, serv_len);
    // 设置同时监听的最大个数
    listen(lfd, 2000);
    //设置监听文件描述符为非阻塞
    int flag=fcntl(lfd,F_GETFL,0);
    fcntl(lfd,F_SETFL,flag|O_NONBLOCK);
}


pair<int,sockaddr_in> TcpListen::acceptConnect()
{
    sockaddr_in client;
    socklen_t len=sizeof(client);
    int cfd=accept(lfd,(sockaddr*)(&client),&len);
    pair<int,sockaddr_in> ret;
    ret.first=-1;
    if(cfd==-1)
    {
        if(errno==EWOULDBLOCK||errno==ECONNABORTED
                ||errno==EPROTO||errno==EINTR)
        {
            return ret;
        }
        else
        {
            cout<<"accept err"<<endl;
            perror("accept err");
            throw bad_alloc();
        }
    }
    ret.first=cfd;
    ret.second=client;
    return ret;
}

void TcpListen::destory()
{
    m_magic=-1;
    close(lfd);
    memset(&ser,0,sizeof(ser));
}

TcpListen::~TcpListen()
{
}

int TcpListen::getFd()
{
    return lfd;
}
