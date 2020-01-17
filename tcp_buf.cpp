#include "tcp_buf.h"

#include "tcp_client.h"

void Client::set_nonblock()
{
    int flag=fcntl(cfd,F_GETFL,0);
    fcntl(cfd,F_SETFL,flag|O_NONBLOCK);
}
Client::Client(string ip,int prot)
{
    this->ip_=ip;
    this->prot_=prot;
    this->cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1)
    {
        perror("create socket err");
    }
}
void Client::nio_connect()
{
    sockaddr_in ser;
    int max_fd;
    socklen_t len=sizeof(ser);
    memset(&ser,0,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(prot_);
    inet_pton(AF_INET,ip_.c_str(),&ser.sin_addr);

    int ret=connect(cfd,(const sockaddr*)(&ser),len);
    fd_set rset,wset;
    bool con=false;
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
        return;
    }
    if(con)
    {
        struct timeval tv;
        tv.tv_sec=5;
        tv.tv_usec=0;
        int ret=select(cfd,&rset,&wset,NULL,&tv);
        if(ret==0)
        {
            cout<<"time out"<<endl;
            close(cfd);
        }
        if(ret==-1)
        {
            cout<<"select err"<<endl;
            exit(0);
        }
        else
        {
            if(FD_ISSET(cfd,&rset)||FD_ISSET(cfd,&wset))
            {
                int err;
                socklen_t err_len=sizeof(err);
                if((getsockopt(cfd,SOL_SOCKET,SO_ERROR,&err,&err_len)<0)||err!=0)
                {
                    cout<<"connect err"<<endl;
                    exit(0);
                }
                cout<<"connect success"<<endl;
                return;
            }
        }
    }
}

int Client::nio_read(char *buf,int len)
{
    int ret=read(cfd,buf,len);
    if(ret<0)
    {
        perror("read err");
    }
    return ret;
}

int Client::nio_write(char *buf,int len)
{

    int ret=write(cfd,buf,len);
    if(ret<0)
    {
        perror("write err");
    }
    return ret;
}


