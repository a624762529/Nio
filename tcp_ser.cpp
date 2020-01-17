#include "tcp_ser.h"


TcpSer::TcpSer()
{

}
TcpSer::TcpSer(int prot)
{
    this->prot=prot;
}

void TcpSer::init_Ser()
{
    this->nio_creat_sock();
    this->nio_bind();
    this->nio_listen();
}

void TcpSer::set_nonblock()
{
    int flag=fcntl(listen_fd,F_GETFL,0);
    fcntl(listen_fd,F_SETFL,flag|O_NONBLOCK);
}

int TcpSer::nio_bind()
{
    sockaddr_in ser_addr;
    ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=htons(this->prot);
    int ret=bind(this->listen_fd,(const sockaddr*)(&ser_addr),sizeof(ser_addr));
    if(ret==-1)
    {
        cout<<"bind err"<<endl;
        perror("bind err");
        exit(0);
    }
    return ret;
}

pair<int,sockaddr_in> TcpSer::nio_accept()
{
    sockaddr_in client;
    socklen_t len=sizeof(client);

    int cfd=accept(listen_fd,(sockaddr*)(&client),&len);
    int flag=fcntl(cfd,F_GETFL,0);
    pair<int,sockaddr_in> ret;

    if(cfd==-1)
    {
        if(errno==EWOULDBLOCK||errno==ECONNABORTED||errno==EPROTO||errno==EINTR)
        {
            return ret;
        }
        else
        {
            cout<<"accept err"<<endl;
            perror("accept err");
            exit(0);
        }
    }
    ret.first=cfd;
    ret.second=client;
    fcntl(cfd,F_SETFL,flag|O_NONBLOCK);
    return ret;
}

int TcpSer::nio_creat_sock()
{
    this->listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1)
    {
        perror("create socket err");
    }
    int opt=1;
       setsockopt(listen_fd,SOL_SOCKET,
                  SO_REUSEADDR|SO_REUSEPORT,
                  &opt,sizeof(int));
    this->set_nonblock();

}

int TcpSer::nio_listen()
{
    int ret=listen(this->listen_fd,250);
    if(ret<0)
    {
        cout<<"listen err"<<endl;
        perror("err");
        exit(0);
    }
    return ret;
}

TcpSer::~TcpSer()
{
    //
}

void TcpSer::destory()
{
    close(this->listen_fd);
}

int TcpSer::getListenFd()
{
    return listen_fd;
}


void TcpSer::setReadcb
              (function<void (EpollNode *cli,void*)> read_fun,void*readarg)
{
    this->m_readcb=read_fun;
    this->arg_read=readarg;
}

void TcpSer::setWritcb
    (function<void (EpollNode *cli,void*)> wri_fun,void* wriarg)
{
    this->m_writcb=wri_fun;
    this->arg_write=wriarg;
}

void TcpSer::setExeccb
    (function<void (EpollNode *cli,void*)> exp_fun,void* exarg)
{
    this->m_expecb=exp_fun;
    this->arg_exp=exarg;
}

bool TcpSer::operator<(const TcpSer& argser)const &
{
    return this->listen_fd<argser.listen_fd;
}

bool TcpSer::operator==(const TcpSer &ser)
{
    return this->listen_fd==ser.listen_fd;
}
