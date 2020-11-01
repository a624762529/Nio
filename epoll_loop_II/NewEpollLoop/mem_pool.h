#pragma once
#include<iostream>
#include "my_mem_pool.h"

using namespace std;
class LinkNode
{
public:
	LinkNode *next; 
};

class Link
{
public:
	Link();  //构造函数
	~Link(); //析构函数
	void insertNode(void *insertNode);
	void* getHead();//获取链表头部信息
	void  clearLink();
	int linkSize();
private:
	LinkNode *head;
	int size;
};




class MemPool
{
public:
	MemPool();
	~MemPool();
	void delloc(void *temp, int size);
	void *alloc(int size);
private:
	void addMemTrash(void *mem);
	bool fillSecPool(); 
	void allocSlot(); 
	void allocToOneSlot(int num);
	bool rellocMemToPool();
	void zeroPool();
	void * getSecMem(int mv);
	void * getMemFromSec(int mv);
	void addSurplusToSlot();
	bool finalStrategicAllocMem();
	int  tranSizeToSlot(int size);
	void *alloc_mem_(int size);
private:
	void relloc(void *temp, int size);
	void dealalloc(char* mem, int size);
private:
	void clear();
	pair<int, int> flick(int num);
private:
	bool final_alloc_mem();
	void * alloc_mem_from_pool_(int mv);
private:
	void* getNodeFromNext(int num, int &n);
	void* getNodeFromNextSlot(int num);
	void alloc_to_list__(int num);
	void* alloc_lar(int size);
private:
	Link m_node[16];
	Link m_trash;
private:
	char *m_SecPool;
	int   m_SecMemPoolSize;
};

 
//另一个   内存池的单例
class ImPlMem
{
	virtual void* alloc(int size) { return nullptr; };
	virtual void delloc(void *p, int size) { return; };
};
class mem_pool_:public ImPlMem
{
public:
	void* alloc(int size);
	void delloc(void *p, int size);
public:
	static MemPool *all;
};

extern MemPool mem_poo;
extern mem_pool_ sig_pool;

