#include "sock_buff.h"

Big_Sock_Buf myBigBuf;

Big_Sock_Buf::Big_Sock_Buf()
{
    //初始化内存
    memset(big_arry,0,sizeof(big_arry));
    //初始化free_box 初始的时候 空闲链块的大小是64*1024
    Free_Node node(0,64*1024);
    free_box.push_back(node);
}

pair<Big_Sock_Buf::Free_Node,bool> Big_Sock_Buf::try_Get_Mem(int statu)
{
    //获取对应的内存块
    lock_guard<mutex> lock(mu);
    pair<Free_Node,bool> ret;
    if(free_box.size()==0)
    {
        ret.second=false; 
        return ret;
    }
    Free_Node *recod=nullptr;
    Free_Node *bigest=nullptr;
    vector<Free_Node>::iterator recod_big;
    for(vector<Free_Node>::iterator it=free_box.begin();it!=free_box.end();it++)
    {
        if((*it).get_free_size()>statu)
        {
            //从头到尾找一个更小的 但是却又满足条件的
            //最后分裂该较大内存碎片
            if(recod==nullptr||recod->get_free_size()>(*it).get_free_size())
            {
                recod=&(*it);
            }
        }
        else if((*it).get_free_size()==statu)
        {
            //移除掉该节点 将该节点的 内存作为记录节点 返回
            pair<Free_Node,bool> ret;
            ret.first=*it;
            ret.second=true;
            free_box.erase(it);
            return ret;
        }
        else
        {
            //如果既没有大的 也没有合适的
            if(recod==nullptr)
            {
                if(bigest==nullptr||bigest->get_free_size()<(*it).get_free_size())
                {
                    bigest=&(*it);
                }
                recod_big=it;
            }
            continue;
        }
    }
    //如果recod==0 说明申请的内存过于庞大 返回最庞大的内存碎片

    if(recod==nullptr)
    {

        if(bigest==nullptr)
        {
            //内存空间里面已经没有可用的内存 返回false
            ret.second=false;
            return ret;
        }
        ret.first=*bigest;
        ret.second=true;
        free_box.erase(recod_big);
        return ret;
    }
    //没有合适的 但是有相对大的内存碎片
    else
    {
        ret.first= recod->breaku(statu);
        ret.second=true;
        return ret;
    }
}

bool Big_Sock_Buf::put_Mem(int begin,int end)
{
     lock_guard<mutex> lock(mu);

     Free_Node node(begin,end);
     if(free_box.size()==0)
     {
         free_box.push_back(node);
         return true;
     }
     vector<Free_Node>::iterator pre;
     bool merge_success_tag=false;
     for(vector<Free_Node>::iterator it=free_box.begin();
         it!=free_box.end();)
     {
        //化繁为简吧 从头到尾的归并
        //如果当前节点
        //使用while循环进行merge

         //如果归并成功
         {
            while((*it).Merge(node))
            {
                //如果归并成功 那就将该节点从空闲缓冲区取出，继续归并，直到到边界状态
                merge_success_tag=true;
                node=*it;
                it=free_box.erase(it);
                if(it==free_box.end())
                {
                    free_box.push_back(node);
                    return true;
                }
            }
            if(merge_success_tag==true)
            {
                free_box.insert(--it,node);
                return true;
            }
         }
        //未归并成功 但是找到了属于自己的位置
        //顺序归并的一种状况
        if(node.start<(*it).start)
        {
            it=free_box.insert(it,node);
            return true;
        }
        it++;
    }
    free_box.push_back(node);
    return true;
}

char *Big_Sock_Buf::getHandler()
{
    return reinterpret_cast<char*>(big_arry);
}




Sock_Buff::Sock_Buff()
{

}

int  Sock_Buff::getFreeSizeMemPool()//获取空闲buf的长度
{
    return (this->buf_len-this->end_nofree_index);
}

int Sock_Buff::getNoFreeSizeBigBuff()
{
    return this->bigSockInfo.no_free_end_index-this->bigSockInfo.no_free_begin_index;
}

int Sock_Buff::getFreeSizeBigBuf()
{
    return this->bigSockInfo.end_index-this->bigSockInfo.no_free_end_index;
}

int Sock_Buff::getNoFreeSizeMemPool()
{
    return this->end_nofree_index-this->begin_nofree_index;
}

void Sock_Buff::initSockBuf(int alloc_from_mem_pool )//第一个参数的意思是从内存池申请的内存长度
                                                         //第二个参数的意思是这次通讯信道的内容
{
    this->alloc_from_mempool=alloc_from_mem_pool;
    this->buf_len=alloc_from_mempool-sizeof(Sock_Buff);
    this->buf=reinterpret_cast<char*>(this)+sizeof(Sock_Buff);
    memset(this->buf,0,buf_len);

    //初始化 非空闲索引开始和结束的地方
    this->begin_nofree_index=0;
    this->end_nofree_index=0;
}

void Sock_Buff::setInfoAllLen(int infolen)
{
    this->judeGetBufFromBuf(infolen);
    this->info_length=infolen;
}


void Sock_Buff::judeGetBufFromBuf(int info_len)
{
    int ret=this->getStata(info_len);
    if(ret==0)
    {
        //不需要申请内存
    }
    else if(ret==40*1024)
    {
        //放置到线程里面
    }
    else
    {
        //需要申请内存 从BigSockBuf中
        this->need_big_buf=true;
        this->needallocfrombigbuf=ret;
        this->in_big_buff=false;
    }
}

inline int Sock_Buff::getStata(int info_len)
{
    //如果发送的信息在30K一下  那就用自身的资源起解决
    //如果发送的信息处于 30K-100K 可尝试调用10×1024的资源量
    //如果发送的信息在  100K-400K 之间 可尝试调用30*1024 的资源量
    //超过400K 那就把它扔到线程池里去
    if(info_len>=30*1024&&info_len<100*1024)
    {
        return 10*1024;
    }
    else if(info_len>=100*1024&&info_len<400*1024)
    {
        return 30*1024;
    }
    else if(info_len>=400*1024)
    {
        return 40*1024;
    }
    else
    {
        return 0;
    }
}

int Sock_Buff::getMemFromBigBuf()
{
    //如果 该网络sock需要从超大内存中申请节点 并且还未申请到节点的话
    //那就 申请好了 并且初始化info
    if(this->need_big_buf&&(!this->in_big_buff))
    {
        pair<Big_Sock_Buf::Free_Node,bool> ret_=
                myBigBuf.try_Get_Mem(this->needallocfrombigbuf);
        if(ret_.second==true)
        {
            //进行初始化
            Big_Sock_Buf::Free_Node node=ret_.first;
            bigSockInfo.alloc_len=node.get_free_size();
            bigSockInfo.start_index=node.start;
            bigSockInfo.end_index=node.end;

            bigSockInfo.no_free_begin_index=0;
            bigSockInfo.no_free_end_index=0;

            in_big_buff=true;

            return node.get_free_size();
        }
        else if(ret_.second==false)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

Sock_Buff::~Sock_Buff()
{
   this->destorySockBuf();
}

inline int Sock_Buff::getLenFromMemPool()
{
    return this->alloc_from_mempool;
}

void Sock_Buff::destorySockBuf()
{
    //归还从超大内存池中申请的内存
    myBigBuf.put_Mem(bigSockInfo.start_index,bigSockInfo.end_index);


    begin_nofree_index=0;
    end_nofree_index=0;

    need_big_buf=false;
    in_big_buff=false;
}

int Sock_Buff::getSizeFromMemPool()
{
    return alloc_from_mempool;
}

void Sock_Buff::sendBackMemToBigMem()
{
    myBigBuf.put_Mem(bigSockInfo.start_index,bigSockInfo.end_index);
    needallocfrombigbuf=0;
    need_big_buf=false;
    in_big_buff=false;
    memset(&this->bigSockInfo,0,sizeof(bigSockInfo));
}

void Sock_Buff::sendOver()
{
    sendBackMemToBigMem();
    this->info_length=0;
}

void Sock_Buff::getFreeFromMemPool(char *&buf,int &len)
{
    buf=&this->buf[this->end_nofree_index];
    len=this->getFreeSizeMemPool();
    return;
}

void Sock_Buff::getFreeFromBigBuf(char *&buf,int &len)
{
    //获取空闲内存
    char *bigbuf_handle= myBigBuf.getHandler();
    buf=&bigbuf_handle[this->bigSockInfo.no_free_end_index];
    len=bigSockInfo.end_index-bigSockInfo.no_free_end_index;
}

void Sock_Buff::getNoFreeFromMemPool(char *&buf,int &len)
{
    buf=&this->buf[this->begin_nofree_index];
    len=this->end_nofree_index-this->begin_nofree_index;
    return;
}

void Sock_Buff::getNoFreeFromBigBuf(char *&buf,int &len)
{
    buf=(&myBigBuf.big_arry[bigSockInfo.no_free_begin_index]);
    len=bigSockInfo.no_free_end_index-bigSockInfo.no_free_begin_index;
    return;
}

void Sock_Buff::moveEndBackMemPool(int len)
{
    this->end_nofree_index+=len;
    return;
}

void Sock_Buff::moveEndBackBigBuf(int len)
{
    this->bigSockInfo.no_free_end_index+=len;
}
//从通讯套接字中读取信息的时候 执行移动非空闲索引

//客户从sockbuf中国读取数据的时候 执行移动空闲索引
void Sock_Buff::moveStartBackMemPool(int len)
{
    this->begin_nofree_index+=len;
    if(begin_nofree_index==end_nofree_index)
    {
        this->begin_nofree_index=0;
        this->end_nofree_index=0;
    }
    return;
}

int  Sock_Buff::getInfoLen()
{
    return info_length;
}

int Sock_Buff::getNofreeSize()
{
    return getNoFreeSizeBigBuff()+getNoFreeSizeMemPool();
}

void Sock_Buff::moveStartBackBigBuf(int len)
{
    bigSockInfo.no_free_begin_index+=len;
    if(bigSockInfo.no_free_begin_index==bigSockInfo.no_free_end_index)
    {
        bigSockInfo.no_free_begin_index=0;
        bigSockInfo.no_free_end_index=0;
    }
    return;
}


bool Sock_Buff::get_need_big_buf()
{
    return need_big_buf;
}

bool Sock_Buff::get_in_big_buff()
{
    return in_big_buff;
}
