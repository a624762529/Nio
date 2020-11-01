//#pragma once
//#include<iostream>
//using namespace std;
//class link
//{
//public:
//	link *next = nullptr;
//	void insert(void* next_)
//	{
//		link* temp = (link*)next_;
//		link* cur_next = temp->next;
//		next = temp;
//		temp->next = cur_next;
//	}
//	void free_node()
//	{
//		this->next = nullptr;
//		link* link_f = (link*)this;
//		free(link_f);
//	}
//};
//
//class my_pool
//{
//private:
//	void *_mem = nullptr;
//	int   _size_left=0;
//	int   _max_size=0;
//	link* _link = nullptr;
//public:
//
//	void* alloc(int size)
//	{
//		return alloc_mem(size);
//	}
//	void delloc(void *p, int size)
//	{
//		return dellocate(p);
//	}
//
//
//	my_pool()
//	{
//		//从512字节开始好了
//		__refill_mem(512);
//	}
//	~my_pool()
//	{
//		__clean();
//	}
//	int  get_limt_value(int size)
//	{
//		int value = _max_size;
//		//以4倍的速率去扩充
//		do
//		{
//			value *= 4;
//		} while (size > value);
//		return value;
//
//	}
//	void *get_mem_(int size)
//	{
//		void* mem_temp = _mem;
//		_size_left -= size;
//		_mem = (void*)((int)mem_temp + size);
//		return mem_temp;;
//	}
//	void *alloc_mem(int size)
//	{
//		if (size <= _size_left)
//		{
//			return get_mem_(size);
//		}
//		else
//		{
//			int get_limit = get_limt_value(size);
//			__refill_mem(get_limit);
//			return get_mem_(size);
//		}
//	}
//	void dellocate(void *mem)
//	{
//		return;
//	}
//private:
//	void __insert_value_to_free(void *buf_)
//	{
//		if (buf_)
//		{
//			if (_link == nullptr)
//			{
//				_link = (link*)buf_;
//			}
//			else
//			{
//				_link->insert(buf_);
//			}
//		}
//	}
//	void __refill_mem(int max_size)
//	{
//		void *mv = malloc(max_size + 4);
//		_mem = (void *)((int)mv+4);
//
//		_max_size = max_size;
//		_size_left = _max_size;
//		//存储堆节点到内存池中
//		__insert_value_to_free(mv);
//	}
//	void __clean()
//	{
//		link *cur = _link;
//		while (cur)
//		{
//			link* temp = cur;
//			cur = cur->next;
//			temp->free_node();
//		}
//		this->_link = nullptr;
//		this->_max_size = 0;
//		this->_size_left = 0;
//		this->_mem = nullptr;
//	}
//};