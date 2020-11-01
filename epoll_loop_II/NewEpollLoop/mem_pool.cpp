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
	//��ʼ�� �������
	m_SecMemPoolSize = 0;
	//Ϊ�ڴ�ط����ڴ� 
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
	//Ϊ�����ڴ������1024*64byte���ڴ���Ƭ
	if (m_SecPool == nullptr)
	{
		throw bad_alloc();
	}
	m_SecMemPoolSize = 1024 * 64;
	addMemTrash(m_SecPool);
	//���ײ��ڵ���뵽����վ��
	m_SecPool = (char *)((int)(m_SecPool)+8);
	return (m_SecPool != nullptr);
}

void MemPool::addMemTrash(void *mem)
{
	//���memΪ��
	if (mem == nullptr)
	{
		return;
	}
	m_trash.insertNode(mem);
}

void MemPool::allocSlot()//���ڴ�۷����ڴ�
{
	//16���ڴ���зֱ����8 - 17*8���ڴ��С
	for (int a = 0; a < 16; a++)
	{
		allocToOneSlot(a);
	}
}

void MemPool::allocToOneSlot(int num)
{
	//�ڶ�Ӧ�Ĳ���������ڴ���
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
		//�ڴ���е����ݳ��� 
		void *tem_ret = m_SecPool;
		m_SecPool = (m_SecPool + mv);
		m_SecMemPoolSize -= mv;
		return tem_ret;
	}
	else
	{
		//�ڴ���е����ݲ���
		//�Ӷ��������ڴ�����pool ���벻��������ѭ������
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

//���¸������ڴ��ע���ڴ�
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

//�ڴ�ʵ�����벻�� ʹ����ѭ���ķ�ʽ����
bool MemPool::finalStrategicAllocMem()
{
	while (!rellocMemToPool())//ֱ�����뵽�ŷ���
	{
	}
	return true;
}

void *MemPool::alloc_mem_(int size)
{
	//������ڴ���ڣ�16*8��  
	if (size > 16*8)
	{
		//Ҫ����Ĵ�С������256
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
		//��Ӧ�Ľڵ��пռ䣬���Ǹ��ڵ����ȡ
		if (m_node[num].linkSize())
			return m_node[num].getHead();

		else
		{
			//����ڴ�ص��ڴ����size ���
			if (m_SecMemPoolSize >= size)
			{
				//�Ӷ����ڴ������ڵ�(�����ܵ�����12���ڵ� ���벻��12��Ҳ��)
				allocToOneSlot(num);
				//�ݹ���������  ���뵽�� �ݹ����� ���еĻ� �͡���
				return alloc_mem_(size);
			}
			else
			{
				//�ڴ����û���ڴ� �Ӹ���Ľڵ��ȡ�ڵ�
				void *temp = getNodeFromNextSlot(num);
				if (!temp)
				{
					//�������12���ڵ�
					allocToOneSlot(num);
					return alloc_mem_(size);
				}
				else
				{
					//������ѭ�� ֱ��malloc���ڵ�
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
	//��num��Ӧ�Ľڵ�֮�� ȡ���۽ڵ�
	void *temp = getNodeFromNext(num, nextsize);
	if (temp == nullptr)
		return temp;
	//���»�ʣ�¶���
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
	while (!rellocMemToPool())//ֱ�����뵽�ŷ���
	{
	}
	return true;
}

void MemPool::relloc(void *temp, int size)
{
	//size <=0 ����
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