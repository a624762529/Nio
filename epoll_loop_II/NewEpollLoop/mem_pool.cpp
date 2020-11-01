#include"mem_pool.h"

MemPool	  mem_poo;
mem_pool_ sig_pool;
MemPool* mem_pool_::all = &mem_poo;

Link::Link()
{
	size = 0;
	head = NULL;
}

Link::~Link()
{
	//this->clearLink();
}

void Link::insertNode(void *insertNode)
{
	if (head == nullptr)
	{
		head = reinterpret_cast<LinkNode*>(insertNode);
		head->next = nullptr;
		 
	}
	else
	{
		LinkNode *head_next = head->next;
		LinkNode *head_ = head;
		head= reinterpret_cast<LinkNode*>(insertNode);
		head->next = head_;
	}
	size++;
}

void* Link::getHead()
{
	if (size == 0)
	{
		return nullptr;
	}
	else
	{
		size--;
		auto ret= head;
		head = head->next;
		return ret;
	}
}

void  Link::clearLink()
{
	//success
	//while (head)
	//{
	//	auto cur = head;
	//	head = head->next;
	//	free(cur);
	//}

	//get
	while (true)
	{
		auto i = getHead();
		if (i == nullptr)
		{
			break;
		}
		delete i;
	}
}

int Link::linkSize()
{
	return size;
}



MemPool::MemPool()
{
	//初始化 各个组件
	m_SecMemPoolSize = 0;
	//为内存池分配内存 
	fillSecPool();
	allocSlot();
}

MemPool::~MemPool()
{
	m_trash.clearLink();
	zeroPool();
}

bool MemPool::fillSecPool()
{
	m_SecPool = (char*)malloc(1024 * 64 + 8);
	//为二级内存池申请1024*64byte的内存碎片
	if (m_SecPool == nullptr)
	{
		throw bad_alloc();
	}
	m_SecMemPoolSize = 1024 * 64;
	addMemTrash(m_SecPool);
	//将首部节点插入到回收站中
	m_SecPool = (char *)((int)(m_SecPool)+8);
	return (m_SecPool != nullptr);
}

void MemPool::addMemTrash(void *mem)
{
	//如果mem为空
	if (mem == nullptr)
	{
		return;
	}
	m_trash.insertNode(mem);
}

void MemPool::allocSlot()//向内存槽分配内存
{
	//16个内存槽中分别放置8 - 17*8的内存大小
	for (int a = 0; a < 16; a++)
	{
		allocToOneSlot(a);
	}
}

void MemPool::allocToOneSlot(int num)
{
	//在对应的槽上面分配内存是
	//8 16 24 32 40 48
	int size = (num + 1) * 8;
	for (int a = 0; a < 12; a++)
	{
		//m_node[num] = (LinkNode*)alloc_mem_from_pool(size);
		m_node[num].insertNode( getMemFromSec(size));
	}
}

void * MemPool::getMemFromSec(int mv)
{
	return getSecMem(mv);
}

void * MemPool::getSecMem(int mv)
{
	if (m_SecMemPoolSize > mv)
	{
		//内存池中的内容充足 
		void *tem_ret = m_SecPool;
		m_SecPool = (m_SecPool + mv);
		m_SecMemPoolSize -= mv;
		return tem_ret;
	}
	else
	{
		//内存池中的内容不够
		//从堆中申请内存填满pool 申请不到陷入死循环死等
		if (rellocMemToPool())
		{
			return getSecMem(mv);
		}
		else
		{
			finalStrategicAllocMem();
			getSecMem(mv);
		}
	}
}

//重新给二级内存池注入内存
bool MemPool::rellocMemToPool()
{
	if (m_SecMemPoolSize != 0)
		addSurplusToSlot();
	zeroPool();
	return fillSecPool();
}

void MemPool::addSurplusToSlot()
{
	int num = m_SecMemPoolSize / 8;
	if (num > 0)
	{
		m_node[num].insertNode(m_SecPool);
	}
}

void MemPool::zeroPool()
{
	m_SecPool = nullptr;
	m_SecMemPoolSize = 0;
}

//内存实在申请不到 使用死循环的方式申请
bool MemPool::finalStrategicAllocMem()
{
	while (!rellocMemToPool())//直到申请到才返回
	{
	}
	return true;
}

void *MemPool::alloc_mem_(int size)
{
	//申请的内存大于（16*8）  
	if (size > 16*8)
	{
		//要申请的大小数大于256
		void *obj = malloc(size+8);
		m_trash.insertNode(obj);
		return reinterpret_cast<char*>(obj)+8;
	}
	if (size <= 0)
	{
		return nullptr;
	}
	else
	{
		int num = tranSizeToSlot(size);
		//对应的节点有空间，从那个节点出获取
		if (m_node[num].linkSize())
			return m_node[num].getHead();

		else
		{
			//如果内存池的内存大于size 填充
			if (m_SecMemPoolSize >= size)
			{
				//从二级内存池申请节点(尽可能的申请12个节点 申请不到12个也行)
				allocToOneSlot(num);
				//递归重新申请  申请到了 递归申请 不行的话 就。。
				return alloc_mem_(size);
			}
			else
			{
				//内存池中没有内存 从更大的节点获取节点
				void *temp = getNodeFromNextSlot(num);
				if (!temp)
				{
					//往上面挂12个节点
					allocToOneSlot(num);
					return alloc_mem_(size);
				}
				else
				{
					//采用死循环 直到malloc出节点
					final_alloc_mem();
					return alloc_mem_(size);
				}
			}
		}
	}
}

void* MemPool::getNodeFromNextSlot(int num)
{
	int nextsize = 0;
	//从num对应的节点之后 取出槽节点
	void *temp = getNodeFromNext(num, nextsize);
	if (temp == nullptr)
		return temp;
	//看下还剩下多少
	int size_left = nextsize - num * 8;
	int retslot = tranSizeToSlot(size_left);
	void *temp_ret = (void*)((int)temp + retslot);
	if (size_left > 8)
	{
		m_node[retslot].insertNode(temp_ret);
	}
	return temp;
}

void* MemPool::getNodeFromNext(int num, int &n)
{
	for (int a = num + 1; a < 16; a++)
	{
		if (m_node[a].linkSize() !=0)
		{
			n = a;
			return m_node[a].getHead();
		}
	}
	return nullptr;
}

int MemPool::tranSizeToSlot(int size)
{
	int num = size / 8;
	if (num == 0)
	{
		return 0;
	}
	if (size%num == 0)
	{
		return num - 1;
	}
	else
	{
		return num;
	}
}

void *MemPool::alloc(int size)
{
	return alloc_mem_(size);
}

bool MemPool::final_alloc_mem()
{
	while (!rellocMemToPool())//直到申请到才返回
	{
	}
	return true;
}

void MemPool::relloc(void *temp, int size)
{
	//size <=0 返回
	if (size <= 0)
		return;
	if (size > 128)
	{
		dealalloc(reinterpret_cast<char*>(temp), size);
		return;
	}
	int num = tranSizeToSlot(size);
	m_node[num].insertNode(temp);
	return;
}

void MemPool::dealalloc(char* mem, int size_)
{
	for (int i = 0; i < 16; i++)
	{
		int quaSlot = m_node[i].linkSize();
		int qua_size = (i + 1) * 8;
		for (int i_ = quaSlot; i_ < 12; i_++)
		{
			m_node[i].insertNode(mem);
			mem += quaSlot;
			size_ -= qua_size;
			if (size_ < qua_size)
			{
				goto ST;
			}
		}
		if (size_ < qua_size + 8)
		{
			break;
		}
	}
	ST:
	auto ret = flick(size_);
	for (int i = 0; i < ret.first; i++)
	{
		m_node[15].insertNode(mem);
		mem += 16 * 8;
		size_ -= 16 * 8;
	}

	int leftsl = tranSizeToSlot(ret.second);
	if (size_ < 8)
	{
		return;
	}
	m_node[leftsl].insertNode(mem);
}

pair<int, int> MemPool::flick(int num)
{
	int num_ = num / 128;
	int lost_ = num % 128;
	return pair<int, int>(num_, lost_);
}

void MemPool::delloc(void *temp, int size)
{
	relloc(temp, size);
}

void* MemPool::alloc_lar(int size)
{
	return malloc(size);
}

void MemPool::alloc_to_list__(int num)
{
	int size = (num + 1) * 8;
	for (int a = 0; a < 12; a++)
	{
		/*if (!m_node[num])
		{
			m_node[num] = (LinkNode*)getMemFromSec(size);
			m_node[num]->next = nullptr;
		}
		else
		{
			m_node[num]->insert_node(getMemFromSec(size));
		}*/
		m_node[num].insertNode(getMemFromSec(size));
	}
}

void MemPool::clear()
{
	m_trash.clearLink();
}

void* MemPool::alloc_mem_from_pool_(int mv)
{
	return getSecMem(mv);
}


void* mem_pool_::alloc(int size)
{
	return all->alloc(size);
}
void mem_pool_::delloc(void *p, int size)
{
	return all->delloc(p, size);
}