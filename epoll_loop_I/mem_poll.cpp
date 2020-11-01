#include "mem_poll.h"

Node::Node()
{
    this->next=nullptr;
}
void Node::add_Node(void *mem)
{
    //采用首插法 讲mem插入到首部节点的next
    Node *now_next=this->next;
    Node *temp=reinterpret_cast<Node*>(mem);
    temp->next=now_next;
    this->next=temp;
}
void *Node::get_Node()
{
    Node *now=reinterpret_cast<Node*>(this);
    return now;
}


Link::Link()
{
    //初始化桶节点信息
    this->head=nullptr;
    this->node_size=0;
}
int Link::now_capsize()
{
    return node_size;
}
void Link::add_Node_To_Link(void *mem)
{
    if(mem==nullptr)
    {
        //如果要插入的节点为空 那就抛出异常
        cout<<__FUNCTION__<<__LINE__<<"参数异常"<<endl;
        throw bad_alloc();
    }
    //当前节点+1
    this->node_size+=1;
    if(head==nullptr)
    {
        head=reinterpret_cast<Node*>(mem);
        head->next=nullptr;
        return;
    }
    head->add_Node(mem);
    return;
}
void* Link::get_Node_From_Link()
{
    //当前没有节点 那就返回null；
    if(head==nullptr)
    {
        return nullptr;
    }
    this->node_size-=1;
    Node *tem=head;
    head=head->next;
    return tem;
}
void Link::clear_Mem()
{
    //清理其中的内存
    while (this->head)
    {
        Node *clr=head;
        head=head->next;
        free(clr);
    }
    return;
}

Mem_Poll::Mem_Poll()
{
    this->mem_sec=nullptr;
    this->now_size=0;
    fillSecPoll();
    fillAllSlot();
}
Mem_Poll::~Mem_Poll()
{
    trash.clear_Mem();
    this->mem_sec=nullptr;
    this->now_size=0;
    zeroSlot();
}

//二级内存池的相关函数实现区域
bool Mem_Poll::fillSecPoll()
{

    //实现思路 在堆中申请40*3072大小的内存
    int alloc_size=40*3072+8;
    void *mem=malloc(alloc_size);
    void *add_mem=(void*)((char*)(mem)+8);

    if(mem==nullptr)
    {
        return false;
    }
    trash.add_Node_To_Link(mem);
    this->now_size=40*3072;
    this->mem_sec=add_mem;
    return true;
}
void Mem_Poll::addSecMemToSlot()
{
    int index=now_size/1024-1;
    poll[index].add_Node_To_Link(mem_sec);
    now_size=0;
}

void *Mem_Poll::getMemFromNextSlot(int index)
{
    //从当前索引的下一个节点获取元素
    for(index;index<3;index++)
    {
        //如果有节点 那就从当前槽中获取元素
        if(poll[index].now_capsize())
        {
            int size=(index+1)*1024;
            this->now_size=size;
            return poll[index].get_Node_From_Link();
        }
    }
    return nullptr;
}
void Mem_Poll::ifSecMemNotEnough(int size)
{
    if(now_size<size)
    {
        if(now_size!=0)
        {
            //将剩余的内存挂到满足它的槽上面
            addSecMemToSlot();
        }
        //重新填充二级内存池
        fillSecPoll();
        //如果填充成功 那就往其中注入内存
        if(now_size==0)
        {
            //内存紧张 那就从他的下一个节点
            //出申请一块较大的内存好了
            int index=size/1024-1;
            if(index==2)
            {
                throw bad_alloc();
            }
            else
            {
                //内存分配失败 那就从
                //5下一个内存槽中获取元素
                //如果后面的槽中没有节点 那就把程序挂掉
                this->mem_sec=
                getMemFromNextSlot(index+1);
                if(this->now_size==0)
                {
                    cout<<"mem err"<<__FUNCTION__
                       <<__LINE__<<endl;
                    throw bad_alloc();
                }
            }
        }
    }
}

void *Mem_Poll::getMemFromSecPoll(int size)
{
    //判断是否二级内存池空间足够
    ifSecMemNotEnough(size);
    //如果二级内存池中资源可用满足需求
    //那就取出元素 移动二级内存池指针的位置
    char *mem=reinterpret_cast<char*>(this->mem_sec);
    char *ret=mem;
    mem+=size;

    this->now_size-=size;
    this->mem_sec=reinterpret_cast<void*>(mem);
    return ret;
}

//针对三个槽区操作的区域
void Mem_Poll::slotReFill(int i)
{
    Link *link=&poll[i];
    int i_size=link->now_capsize();
    int block_size=(i+1)*1024;
    for(i_size;i_size<12;i_size++)
    {
        link->add_Node_To_Link
                (getMemFromSecPoll(block_size));
    }
}
void Mem_Poll::fillAllSlot()
{
    for(int i=0;i<3;i++)
    {
        slotReFill(i);
    }
}
void Mem_Poll::zeroSlot()
{
    for(int i=0;i<3;i++)
    {
        poll[i].head=nullptr;
        poll[i].node_size=0;
    }
}
void *Mem_Poll::getMemFromSlot(int size)
{
    int index=size/1024-1;
    //如果申请的内存在1024-3072 代表索引合法
    if(index>=0&&index<3)
    {
        again:
        Link *lk=&poll[index];
        if(lk->now_capsize())
        {
            return lk->get_Node_From_Link();
        }
        else
        {
            //如果当前槽中没有节点了
           slotReFill(index);
           goto again;
        }
    }
    return nullptr;
}
void Mem_Poll::putMemToSlot(const void *mem,int size)
{
    int index=size/1024-1;
    poll[index].add_Node_To_Link(const_cast <void*>(mem));
    return;
}
void* Mem_Poll::get_Mem_From_Slot(int size)
{
    return getMemFromSlot(size);
}
void Mem_Poll::put_Mem_To_Slot(const void *mem,int size)
{
    return putMemToSlot(mem,size);
}


Mem_Poll __mem_pool;
Mem_Poll* Sig_Pool::mem_pool=nullptr;

Sig_Pool alloc;
Sig_Pool::Sig_Pool()
{
    this->mem_pool=&__mem_pool;
}
Sig_Pool::~Sig_Pool()
{}
void *Sig_Pool::alloc_Mem(int size)
{
    lock_guard<mutex> lock(mu);
    return mem_pool->get_Mem_From_Slot(size);
}
void Sig_Pool::put_Mem(int size,void *mem)
{
    lock_guard<mutex> lock(mu);
    mem_pool->put_Mem_To_Slot(mem,size);
    return;
}
