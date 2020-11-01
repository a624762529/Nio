#include "tcp_client.h"


//TCP Client 实现区域
TcpClient::TcpClient(string ip,int prot,int cfd)
{
    initInfo(ip,prot,cfd );
}

TcpClient::TcpClient()
{
}

void TcpClient::initInfo(string ip,int prot,int cfd )
{
    this->ip_=ip;
    this->prot_=prot;
    this->cfd_=cfd;
    //this->allocfrompoollen=allocpoollen;
    //this->sock_buf=(Sock_Buff*) (reinterpret_cast<char*>(this)+sizeof(TcpClient));
    //sock_buf->initSockBuf(allocpoollen-sizeof(TcpClient));
    sock_buf=nullptr;
    this->assist.initAss();
}



TcpClient::TcpClient(const TcpClient &&cli)
{
    this->assist=cli.assist;
    this->sock_buf=cli.sock_buf;
    this->cfd_=cli.cfd_;
    this->ip_=cli.ip_;
    memset( (void*)(&cli),0,sizeof(TcpClient));
}

//TcpClient::~TcpClient()
//{
//    if(sock_buf!=nullptr)
//    {
//        this->judeRWOver();
//    }
//}

void TcpClient::initSockBuf(int size)
{
    if(sock_buf!=nullptr)
    {
        if(this->sock_buf->getSizeFromMemPool()>=size)
        {
            return;
        }
        else
        {
            //当前的内存空间不够 那就...归还原先的内存空间块
            alloc.put_Mem(this->sock_buf->getSizeFromMemPool(),
                          reinterpret_cast<void*>(this->sock_buf));
            sock_buf=nullptr;

        }
    }
    this->sock_buf=reinterpret_cast<Sock_Buff*>
            (alloc.alloc_Mem(size));
    sock_buf->initSockBuf(size);
}

void TcpClient::initSockBufMemPool(int size)
{
    initSockBuf(size);
}

void TcpClient::setInfoAllLen(int len)
{
    this->sock_buf->setInfoAllLen(len);
    assist.initAss();
}

void TcpClient::setAssistInfo(bool cur_,int fd_len,int status)
{
    //this->assist=AssistInfo(cur_,fd_len,status);
}

void TcpClient::setAssisStatu(int statu)
{
    assist.statu=statu;
}

bool TcpClient::judeRWOver()
{
    return (assist.fd_len>=sock_buf->getNofreeSize());
}

bool TcpClient::judeCurOver()
{
   return (assist.fd_len>=this->sock_buf->getInfoLen());
}

void TcpClient::doRWOver()
{
    //将超大内存块申请的节点归还
    sock_buf->sendBackMemToBigMem();
    //将辅助数据中的内容初始化
    //assist.fd_len=0;
    assist.destory();

}

TcpClient::~TcpClient()
{
    this->destory();
}

void TcpClient::destory()
{
    if(sock_buf==nullptr)
    {
        return;
    }
    sock_buf->destorySockBuf();
    alloc.put_Mem(this->sock_buf->getSizeFromMemPool(),
                  reinterpret_cast<void*>(this->sock_buf));
    sock_buf->alloc_from_mempool=0;
    memset(&this->assist,0,sizeof(AssistInfo));
    close(cfd_);
}

void TcpClient::sendBackBigBuf()
{
    if(sock_buf==nullptr)
    {
        return;
    }
    sock_buf->sendBackMemToBigMem();
    memset(&this->assist,0,sizeof(AssistInfo));
}


void TcpClient::getFromBigBuf()
{
    this->sock_buf->getMemFromBigBuf();
}

int TcpClient::getFreeSize()
{
    this->sock_buf->getFreeSizeBigBuf()+this->sock_buf->getFreeSizeMemPool();
}
int TcpClient::nio_write()
{
    if(assist.can_write==cannotcontwr)
    {
        return 0;
    }
    char *readarry=nullptr;
    int arrylen=0;
    if(assist.read_cur==InMemPool)
    {
        sock_buf->getNoFreeFromMemPool(readarry,arrylen);
        if(arrylen>0)
        {
            int ret=write(this->cfd_,readarry,arrylen);
            if(ret==-1)
            {
                if(errno!=EWOULDBLOCK)
                {
                    perror("write err");
                    cout<<__LINE__<<__FUNCTION__<<endl;
                    throw bad_alloc();
                    return -1;
                }
                else
                {
                    //将之挂在epoll树的等待可写的地方
                    return -1;
                }
            }
            else
            {
                assist.fd_len+=ret;
                sock_buf->moveStartBackMemPool(ret);
                //mempool申请的内存空间已经被全部写入到文件描述符中
                if(sock_buf->getNoFreeSizeMemPool()==0)
                {
                    //进而将sockbuf中的内容写入到文件描述符中
                    assist.read_cur=InBigBuf;
                }
            }
        }
    }
    //如果超大内存空间尚有空间
    if(sock_buf->get_in_big_buff()==true)
    {

        sock_buf->getNoFreeFromBigBuf(readarry,arrylen);
        if(arrylen>0)
        {

            int ret=write(this->cfd_,readarry,arrylen);
            if(ret==0)
            {
                return 0;
            }
            else if(ret==-1)
            {
                if(errno!=EWOULDBLOCK)
                {
                    perror("write err");
                    cout<<__LINE__<<__FUNCTION__<<endl;
                    throw bad_alloc();
                    return -1;
                }
                else
                {
                    //挂在等待可写部分
                    return -1;
                }
            }
            else
            {
                sock_buf->moveStartBackBigBuf(ret);
                if(this->sock_buf->getNoFreeSizeBigBuff()==0)
                {
                    //如果bigbuf中没有非空闲区间的时候 那就
                    assist.read_cur=InMemPool;
                    assist.write_cur=InMemPool;
                    assist.can_write=cannotcontwr;
                    assist.can_read=cancontwr;
                }
            }
        }
    }
    else
    {
        assist.read_cur=InMemPool;
        if(sock_buf->get_need_big_buf())
        {
            if(sock_buf->get_in_big_buff()==false)
            {
                sock_buf->getMemFromBigBuf();
            }
        }
    }
    return 1;
}



int TcpClient::nio_read()
{
    if(assist.can_read==cannotcontwr)
    {
        return -2;
    }
    char *readarry=nullptr;
    int arrylen=0;
    //获取从内存池中获取的空闲内存
    //如果当前的游标处于内存池
    if(this->assist.read_cur==InMemPool)
    {
        sock_buf->getFreeFromMemPool(readarry,arrylen);
        if(arrylen>0)
        {
            //将通讯文件描述符的东西写入到从内存池中申请空闲内存中
            int ret=read(this->cfd_,readarry,arrylen);
            if(ret==0)
            {
                //关闭通讯文件描述符
                //..
                return 0;
            }
            else if(ret>0)
            {
                //移动 non_free_end节点
                //fd_len记载已经从文件描述符中读取的数据

                sock_buf->moveEndBackMemPool(ret);
                //如果从内存池申请的内存还被用完 说明文件描述符中还有数据 那就让它读取bigbuf中的信息
                if(sock_buf->getFreeSizeMemPool()==0)
                {
                    //内存池  还有空间
                    this->assist.read_cur=InBigBuf;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                if(errno!=EWOULDBLOCK)
                {
                    perror("read err");
                    cout<<__LINE__<<__FUNCTION__<<endl;
                    throw bad_alloc();
                    return -3;
                }
                return -1;
            }

        }
    }


    //如果获取到了内存 那就。。。
    if(sock_buf->get_in_big_buff())
    {
        this->assist.read_cur=InBigBuf;
        //超大内存块中有数据
        sock_buf->getFreeFromBigBuf(readarry,arrylen);
        if(arrylen>0)
        {
            int ret=read(this->cfd_,readarry,arrylen);
            if(ret==0)
            {
                cout<<"链接中断"<<this->ip_<<endl;
                //执行关闭连接操作
                return 0;
            }
            else if(ret==-1)
            {
                //判断文件是否读取结束读取结束的话 归还超大内存块
                if(errno!=EWOULDBLOCK)
                {
                    perror("read err");
                    cout<<__LINE__<<__FUNCTION__<<endl;
                    throw bad_alloc();
                    return -3;
                }
                return -1;
            }
            else
            {
                sock_buf->moveEndBackBigBuf(ret);
                if(sock_buf->getFreeSizeBigBuf()==0)
                {
                    assist.can_read=cannotcontwr;
                    assist.can_write=cancontwr;
                    assist.read_cur=InMemPool;
                    assist.write_cur=InMemPool;
                }
            }
        }
    }
    else
    {
        assist.read_cur=InMemPool;
        if(sock_buf->get_need_big_buf())
        {
            if(sock_buf->get_in_big_buff()==false)
            {
                sock_buf->getMemFromBigBuf();
            }
        }
        else
        {
            assist.can_read=cannotcontwr;
            assist.can_write=cancontwr;
            assist.read_cur=InMemPool;
        }
    }
    return 1;
}

//_cli 是客户和 sock_buf的

int TcpClient::read_cli(char *arry,int len_r)
{
    //在此处 做出判断 判断信息是否读取结束
    //如果信息读取完毕 那就进行收尾工作  信息未读取完毕 返回未读取完毕的信息
    int f_sz=sock_buf->info_length - assist.fd_len;

    {
        int len=min(len_r,f_sz);
        if(sock_buf->getNoFreeSizeBigBuff()+
                sock_buf->getNoFreeSizeMemPool()==0)
        {
            return 0;
        }
        char *mem_read=nullptr;
        int mempoollen=0;
        int len_=len;
        if(assist.write_cur==InMemPool)
        {
            this->sock_buf->getNoFreeFromMemPool(mem_read,mempoollen);
            //如果客户端要读取的长度大于mempoollen
            if(len_>mempoollen)
            {
                memcpy(arry,mem_read,mempoollen);
                sock_buf->moveStartBackMemPool(mempoollen);
                len_=len_-mempoollen;
                arry+=mempoollen;
                assist.write_cur=InBigBuf;
            }
            else
            {
                //如果要读取的长度小于内存池的申请的长度 那就读取完了返回就行了
                memcpy(arry,mem_read,len_);
                sock_buf->moveStartBackMemPool(len_);
                assist.fd_len+=len;
                return len;
            }
        }

        //如果从超大内存空间申请到了元素
        if(sock_buf->get_need_big_buf()==true)
        {
            int bigmemlen=0;
            this->sock_buf->getNoFreeFromBigBuf(mem_read,bigmemlen);
            //如果参数中的len长度大于超大内存空间的长度
            if(len_>bigmemlen)
            {
                memcpy(arry,mem_read,bigmemlen);
                //清空内存池中的数据
                sock_buf->moveStartBackBigBuf(bigmemlen);
                len_=len_-bigmemlen;
                arry+=bigmemlen;
                assist.can_read=cancontwr;
                assist.fd_len+=mempoollen+bigmemlen;
                return mempoollen+bigmemlen;
            }
            else
            {
                memcpy(arry,mem_read,len_);
                sock_buf->moveStartBackBigBuf(len_);
                assist.fd_len+=len;
                return len;
            }
        }
        else
        {
            assist.write_cur=InMemPool;
            assist.can_read=cancontwr;
            assist.fd_len+=mempoollen;
            return mempoollen;
        }
    }
}

/*
    将参数的内容写入到sockbuf中
*/

int TcpClient::write_cli(char *arry,int len_w)
{
    //将参数中的buf写入到Sockbuf中
    int f_sz=sock_buf->info_length - assist.fd_len;
    {
        int len=min(len_w,f_sz);
        if(sock_buf->getFreeSizeBigBuf()+sock_buf->getFreeSizeMemPool()==0)
        {
            return 0;
        }
        char *mem_read=nullptr;
        int mempoolfreelen=0;
        int len_=len;
        //如果游标在内存池中
        if(assist.write_cur==InMemPool)
        {
            this->sock_buf->getFreeFromMemPool(mem_read,mempoolfreelen);
            if(mempoolfreelen>len_)
            {
                memcpy(mem_read,arry,len_);
                this->sock_buf->moveEndBackMemPool(len_);
                assist.fd_len+=len_;
                return len_;
            }
            else if(mempoolfreelen<=len_)
            {
                memcpy(mem_read,arry,mempoolfreelen);
                this->sock_buf->moveEndBackMemPool(mempoolfreelen);
                arry+=mempoolfreelen;
                assist.write_cur=InBigBuf;
            }
        }
        if(sock_buf->get_in_big_buff())
        {
            //如果内存池中有数据的话 那就
            int big_buf_len=0;
            this->sock_buf->getFreeFromBigBuf(mem_read,big_buf_len);

            if(len_>big_buf_len)
            {
                memcpy(mem_read,arry,big_buf_len) ;
                sock_buf->moveEndBackBigBuf(big_buf_len);
                assist.can_write=cancontwr;
                assist.fd_len+=big_buf_len+mempoolfreelen;
                return big_buf_len+mempoolfreelen;
            }
            else
            {
                memcpy(mem_read,arry,len_) ;
                sock_buf->moveEndBackBigBuf(len_);
                assist.fd_len+=len;
                return len;
            }
        }
        else
        {
            assist.write_cur=InMemPool;
            if(sock_buf->get_need_big_buf())
            {
                if(sock_buf->get_in_big_buff()==false)
                {
                    sock_buf->getMemFromBigBuf();
                }
            }
        }
        assist.fd_len+=mempoolfreelen;
        return mempoolfreelen;
    }
}


int TcpClient::read_head()
{

    char arry[4];
    int len=4;
    //获取非空闲块的内存

    if(this->sock_buf)
    {
        int pool_result=sock_buf->getNoFreeSizeMemPool();
        if(pool_result>0)
        {
            int len_mempool=0;
            char*mem_mempool=nullptr;
            sock_buf->getNoFreeFromMemPool(mem_mempool,len_mempool);


            len_mempool=min(len,len_mempool);
            memcpy(arry,mem_mempool,len_mempool);
            sock_buf->moveStartBackMemPool(len_mempool);
            len-=len_mempool;
        }
        if(this->sock_buf->in_big_buff)
        {
            if(len)
            {
                int len_bigbuf=0;
                char*mem_bigbuf=nullptr;
                sock_buf->getNoFreeFromMemPool(mem_bigbuf,len_bigbuf);

                int index=sizeof(int)-len;
                len_bigbuf=min(len,len_bigbuf);
                memcpy(&arry[index],mem_bigbuf,len_bigbuf);

                sock_buf->moveStartBackBigBuf(len_bigbuf);
                len-=len_bigbuf;
            }
        }
    }
    if(len)
    {
        //剩余部分在文件描述符中
        char buf[4];
        int ret=read(cfd_,buf,len);
        if(ret==-1)
        {
            //不知道咋处理了
            if(errno!=EWOULDBLOCK)
            {
                perror("read err");
                cout<<__LINE__<<__FUNCTION__<<endl;
                throw bad_alloc();
                return -3;
            }
            return -1;
        }
        else if(ret==0)
        {
            //客户端断开链接
            return ret;
        }
        else
        {
            memcpy(&arry[sizeof(int)-len],buf,len);
        }
    }
    return *(int*)arry;
}

int TcpClient::read_head(char *arry,int len)
{
    //len　还需要往对应内存中拷贝多少元素
    //read_len　当前对应的索引部分
    int read_len=0;
    if(this->sock_buf)
    {
        //获取内存池中的空闲内存
        int pool_result=sock_buf->getNoFreeSizeMemPool();
        if(pool_result>0)
        {
            int len_mempool=0;
            char*mem_mempool=nullptr;
            sock_buf->getNoFreeFromMemPool(mem_mempool,len_mempool);

            len_mempool=min(len,len_mempool);
            memcpy(arry,mem_mempool,len_mempool);
            sock_buf->moveStartBackMemPool(len_mempool);
            len-=len_mempool;
            read_len+=len_mempool;
        }

        if(this->sock_buf->in_big_buff)
        {
            if(len)
            {
                int len_bigbuf=0;
                char*mem_bigbuf=nullptr;
                sock_buf->getNoFreeFromMemPool(mem_bigbuf,len_bigbuf);
                int index=read_len;

                len_bigbuf=min(len,len_bigbuf);
                memcpy(&arry[index],mem_bigbuf,len_bigbuf);

                sock_buf->moveStartBackBigBuf(len_bigbuf);
                len-=len_bigbuf;
            }
        }
    }
    if(len)
    {
        //剩余部分在文件描述符中
        char *buf=arry+read_len;
        //将cfd_的内容拷贝到buf中
        int ret=read(cfd_,buf,len);
        if(ret==-1)
        {
            //不知道咋处理了
            if(errno!=EWOULDBLOCK)
            {
                perror("read err");
                cout<<__LINE__<<__FUNCTION__<<endl;
                throw bad_alloc();
                return -3;
            }

            return -1;
        }
        else if(ret==0)
        {
            //客户端断开链接
            return ret;
        }
        else
        {
            memcpy(&arry[read_len],buf,len);
        }
    }
    return read_len;
}
