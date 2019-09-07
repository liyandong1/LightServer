#ifndef _OBJECTPOOL_H_
#define _OBJECTPOOL_H_

#include <mutex>
#include <stdlib.h>
#include <assert.h>

#include "MemoryControl.h"

template <typename T, size_t objectPoolSize>
class ObjectPool
{
private:
	//����ͷ���ṹ
	struct ObjectNodeHeader
	{
	public:
		
		ObjectNodeHeader* pNext_; //��һ���ڵ�λ��
		size_t objectId_;     
		char objectRef_;    //���ô���
		bool isInObjectPool_;
	private:
		char c1;
		char c2;
	};

	ObjectNodeHeader* pHead_;
	char* pBuf_;  //������׵�ַ
	std::mutex m_mutex;

	//��ʼ�������
	void initObjectPool()
	{
		assert(pBuf_ == nullptr);
		size_t allSize = (sizeof(T) + sizeof(ObjectNodeHeader)) * objectPoolSize;
		//����ʹ��new���������ڴ�������ڴ�
		pBuf_ = new char[allSize];

		//����������
		pHead_ = reinterpret_cast<ObjectNodeHeader*>(pBuf_);
		pHead_->pNext_ = nullptr;
		pHead_->objectId_ = 0;
		pHead_->objectRef_ = 1;
		pHead_->isInObjectPool_ = true;

		ObjectNodeHeader* pTail = pHead_;
		for (int i = 1; i < objectPoolSize; i++)
		{
			ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(pBuf_ + i*(sizeof(ObjectNodeHeader) + sizeof(T)));
			pCurrent->pNext_ = nullptr;
			pCurrent->objectId_ = 1;
			pCurrent->objectRef_ = 1;
			pCurrent->isInObjectPool_ = true;

			pTail->pNext_ = pCurrent;
			pTail = pTail->pNext_;
		}
	}
public:
	//const int n = sizeof(ObjectNodeHeader

	ObjectPool()
	{
		pBuf_ = nullptr;
		pHead_ = nullptr;
		initObjectPool();
	}
	
	//��������ڴ�
	void* allocMemory(size_t size)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		assert(pBuf_ != nullptr);
			
		if (pHead_ != nullptr)//�Ӷ�������Ҷ���
		{
			ObjectNodeHeader* pCurrent = pHead_;
			pHead_ = pHead_->pNext_;
			return reinterpret_cast<char*>(pCurrent) + sizeof(ObjectNodeHeader);
		}
		else
		{
			//�������ڴ�������ڴ�
			char* p = new char[sizeof(T) + sizeof(ObjectNodeHeader)];
			ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(p);
			pCurrent->pNext_ = nullptr;
			pCurrent->objectId_ = 0;
			pCurrent->objectRef_ = 1;
			pCurrent->isInObjectPool_ = true;
			return reinterpret_cast<char*>(pCurrent) + sizeof(ObjectNodeHeader);
		}
	}
	//���ն����ڴ�,һ��Ҫע��ƫ������
	void deAllocMemory(void* p)
	{
		char* q = reinterpret_cast<char*>(p) - sizeof(ObjectNodeHeader);
		ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(q);
		assert(pCurrent->objectRef_ == 1);
		pCurrent->objectRef_ = 0;
		if (pCurrent->isInObjectPool_ == true)
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			pCurrent->pNext_ = pHead_;//ͷ�巨
			pHead_ = pCurrent; 
		}
		else
		{
			delete[] q;
		}
	}

	~ObjectPool()
	{
		if(pBuf_ != nullptr)
			delete[] pBuf_;
	}
};



//�ṩһ�����Ľӿڣ�����Ҫʹ�ö���صĶ���̳д˻���
template <typename T, size_t objectPoolSize>
class ObjectPoolBase
{
private:
	static ObjectPool<T, objectPoolSize>& getObjectPool()
	{
		//��̬����ض���
		static ObjectPool<T, objectPoolSize> objPool_;
		return objPool_;
	}
public:
	void* operator new(size_t size)
	{
		return getObjectPool().allocMemory(size);
	}

	void operator delete(void* p)
	{
		getObjectPool().deAllocMemory(p);
	}

	//�ṩ����������������ֱ�ӱ�¶�ӿڣ�������һЩ��������

	//�����Ĺ��캯���ж��������ͳһ��ʹ�ñ��ģ��
	template <typename ...Args>
	static T* createObject(Args... args)
	{
		return new ObjectPoolBase(args...);
	}

	static void destoryObject(T* obj)
	{
		delete obj;
	}
};

#endif