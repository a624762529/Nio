#include "sockclient.h"


SockClient::SockClient(string ip,int prot)
{
    bzero(&addr,sizeof(addr));
    createSocket(ip,prot);
    m_starAlarm=false;
}

SockClient::SockClient()
{
    cfd=-1;
    m_starAlarm=false;
}

int SockClient::connectToHost()
{
    connectToSer();
    return 1;
}

int SockClient::connectToSer()
{
    /*
        tcp链接 成功返回1
        失败 -1
    */
    setNoBlock();
    socklen_t len=sizeof(addr);
    int ret=connect(cfd,(const sockaddr*)(&addr),len);
    fd_set rset,wset;

    bool con=false;
    int max_fd=0;
    int opt=1;
    if(ret<0)
    {
        if(errno!=EINPROGRESS)
        {
            perror("connect err");
            exit(0);
        }
        else
        {
            con=true;
            FD_SET(cfd,&rset);
            FD_SET(cfd,&wset);
            max_fd=cfd;
        }
    }
    else
    {
        setsockopt(cfd,SOL_SOCKET,
                          SO_REUSEADDR|SO_REUSEPORT,
                          &opt,sizeof(int));
        setBlock();
        return 1;
    }
    if(con)
    {
        struct timeval tv;
        tv.tv_sec=5;
        tv.tv_usec=0;
        int ret=select(cfd+1,&rset,&wset,NULL,&tv);
        if(ret==0)
        {
            cout<<"time out"<<endl;
            close(cfd);
            return -1;
        }
        if(ret==-1)
        {
            cout<<"select err"<<endl;
            throw bad_alloc();
        }
        else
        {
            if(FD_ISSET(cfd,&rset)||FD_ISSET(cfd,&wset))
            {
                int err=1;
                socklen_t err_len=sizeof(err);
                int ret=getsockopt(cfd,SOL_SOCKET,SO_ERROR,&err,&err_len);
                if(ret==-1)
                {
                    perror("con error");
                    bad_alloc();
                }

                if( err!=0)
                {
                    cout<<"connect err"<<endl;
                    throw bad_alloc();
                }
                cout<<"connect success"<<endl;
            }
        }
    }
    setsockopt(cfd,SOL_SOCKET,
                      SO_REUSEADDR|SO_REUSEPORT,
                      &opt,sizeof(int));
    setBlock();

    return 1;
}


int SockClient::readInfo(char *buf,int len)
{
    int ret=read(cfd,buf,len);
    if(ret<0)
    {
        if(errno!=EAGAIN)
        {
            perror("read error");
            bad_alloc();
        }
    }
    else if(ret==0)
    {
        cout<<"服务端关闭链接"<<endl;
        destory();
    }
    return len;
}

int SockClient::writeInfo(char *buf, int len)
{
    //客户端给服务端发数据最多就那几个byte..就这样吧
    int readywrite=0;
    while (true)
    {
        int ret=send(cfd,buf,len,MSG_WAITALL);
        readywrite+=ret;
        if(ret==-1&&errno==EAGAIN)
        {
            continue;
        }
        else if(readywrite>=len)
        {
            return ret;
        }
        else
        {
            perror("read error");
            throw bad_alloc();
        }
    }
}

void SockClient::destory()
{
    close(cfd);
    cfd=-1;
    m_starAlarm=false;
}

int SockClient::createSocket(string ip,int prot)
{
    // 创建套接字
    bzero(&addr,sizeof(addr));
    cfd = socket(AF_INET, SOCK_STREAM, 0);


    int opt = 1;
    // sockfd为需要端口复用的套接字
    setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&opt, sizeof(opt));
    // 连接服务器
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(prot);
    inet_pton(AF_INET,ip.c_str() , &serv.sin_addr.s_addr);
    addr= serv;
    return 1;
}

void SockClient::setNoBlock()
{
    int flag=fcntl(cfd,F_GETFL,0);
    fcntl(cfd,F_SETFL,flag|O_NONBLOCK);
}

void SockClient::setBlock()
{
    int flag=fcntl(cfd,F_GETFL,0);
    flag &= ~O_NONBLOCK;
    fcntl(cfd,F_SETFL,flag);
}

void SockClient::setIp(string ip)
{
   inet_pton(AF_INET,ip.c_str() , &addr.sin_addr.s_addr);
}

void SockClient::setProt(int prot)
{
    addr.sin_port = htons(prot);
}

void SockClient::startAlarm()
{
    signal(SIGPIPE,SIG_IGN);
    while (m_starAlarm)
    {
        send(cfd,"a",1,MSG_OOB);
        sleep(4);
    }
}

SockClient::~SockClient()
{
    destory();
    m_starAlarm=false;
}

void SockClient::start()
{
    if(m_starAlarm==false)
    {
        m_starAlarm=true;
        thread th(&SockClient::startAlarm,this);
        th.detach();
    }
}

int  SockClient::stableRecv(char *buf,int len)
{
    //读取对应的长度
    while (true)
    {
        int ret=recv(cfd,buf,len,MSG_WAITALL);
        if(ret==-1&&errno==EAGAIN)
        {
            continue;
        }
        else if(ret>0){
            return ret;
        }
        else
        {
            perror("read err");
            break;
            //throw bad_alloc();
        }
    }
}

