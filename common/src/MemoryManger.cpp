#include "MemoryManger.h"
#include <assert.h>
#include <stdlib.h>

//#include <iostream>
//using namespace std;

/*******************************************************************
						MemoryPool
*/

void MemoryPool::initMemoryPool()
{
	assert(pBuf_ == nullptr);
	//����ͷ��Ϣ�����������ڴ����Ϣ
	size_t allSize = blockCount_ * (sizeof(MemoryBlockHead) + currentMemBlockSize_);
	//��ϵͳ�����ڴ�
	pBuf_ = reinterpret_cast<char*>(malloc(allSize));
	//cout << "pBuf_ = " << (void*)pBuf_<< endl;
	//��ÿ���ڴ�����ͷ����Ϣ
	pHead_ = reinterpret_cast<MemoryBlockHead*>(pBuf_);
	//cout << "PHead_ = " << (void*)pHead_ << endl;
	pHead_->blockId_ = 0;
	pHead_->blockRef_ = 1;
	pHead_->isInMemoryPool_ = true;
	pHead_->ownerMemoryPool_ = this;
	pHead_->pNext_ = nullptr;

	MemoryBlockHead* pTail_ = pHead_;
	for (size_t i = 1; i < blockCount_; i++)
	{
		MemoryBlockHead* pCurrent = reinterpret_cast<MemoryBlockHead*>(pBuf_ + (sizeof(MemoryBlockHead) + currentMemBlockSize_) * i);
		pCurrent->blockId_ = i;
		pCurrent->blockRef_ = 1;
		pCurrent->isInMemoryPool_ = true;
		pCurrent->ownerMemoryPool_ = this;
		pCurrent->pNext_ = nullptr;

		pTail_->pNext_ = pCurrent;
		pTail_ = pTail_->pNext_;
	}
}

MemoryPool::MemoryPool()
{
	pBuf_ = nullptr;
	pHead_ = nullptr;
}

//�����ȥ���ڴ���Ϣ�������û����õ�,ע�����ƫ����
void* MemoryPool::alloMemory(size_t size)
{
	std::lock_guard<std::mutex> lk(m_mutex);
	if (pBuf_ == nullptr)
	{
		initMemoryPool();
	}
	assert(pBuf_ != nullptr);
	//�ڴ�������ڴ��
	if (pHead_ != nullptr)
	{
		MemoryBlockHead* pBlock = pHead_;
		pHead_ = pHead_->pNext_;
		//cout << pBlock->blockId_ << endl;
		return (reinterpret_cast<char*>(pBlock) + sizeof(MemoryBlockHead));
	}
	//�ڴ����û���ڴ��,ֱ�������ϵͳ����
	MemoryBlockHead* pBlock = reinterpret_cast<MemoryBlockHead*>(malloc(sizeof(MemoryBlockHead) + size));
	pBlock->blockId_ = 0;
	pBlock->blockRef_ = 1;
	pBlock->isInMemoryPool_ = false;
	pBlock->ownerMemoryPool_ = nullptr;
	pBlock->pNext_ = nullptr;

	return (reinterpret_cast<char*>(pBlock) + sizeof(MemoryBlockHead));
}

//ע�����ƫ�����Ժ����Ӧ�ù黹��ϵͳ���ڴ�
void MemoryPool::deAllocMemory(void* p)
{
	char* q = reinterpret_cast<char*>(p) - sizeof(MemoryBlockHead);
	MemoryBlockHead* pBlock = reinterpret_cast<MemoryBlockHead*>(q);
	//cout << pBlock->blockId_ << endl;
	if (pBlock->isInMemoryPool_ == true)//���ڴ����
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		if ((--pBlock->blockRef_) != 0)
			return;
		pBlock->pNext_ = pHead_;
		pHead_ = pBlock;    //ͷ�巨�黹�����ڴ��

		//cout << "pBlock=" << pBlock << endl;
	}
	else
	{
		free(pBlock);
	}
}

MemoryPool::~MemoryPool()
{
	if (pBuf_ != nullptr)
		free(pBuf_);
}

/**********************************************************************
							MemoryManger
*/
MemoryManger& MemoryManger::getInstance()
{
	static MemoryManger singletonObject;
	return singletonObject;
}

void MemoryManger::createMapRelatition(size_t begin, size_t end, MemoryPool* pMem)
{
	for (size_t i = begin; i <= end; i++)
		memoryPoolMap[i] = pMem;
}

MemoryManger::MemoryManger()
{
	createMapRelatition(1, 64, &mem64_);
	createMapRelatition(65, 128, &mem128_);
	createMapRelatition(129, 256, &mem256_);
	createMapRelatition(257, 512, &mem512_);
	createMapRelatition(513, 1024, &mem1024_);
}


//�����ڴ�
void* MemoryManger::allocMemory(size_t size)
{
	if (size <= MAX_MEMORY_SIZE)
	{
		return memoryPoolMap[size]->alloMemory(size);
	}

	//ֱ�������ϵͳ���룬ҲҪ����ͷ��Ϣ
	MemoryBlockHead* pBlock = reinterpret_cast<MemoryBlockHead*>(malloc(sizeof(MemoryBlockHead) + size));
	pBlock->blockId_ = 0;
	pBlock->blockRef_ = 1;
	pBlock->isInMemoryPool_ = false;
	pBlock->ownerMemoryPool_ = nullptr;
	pBlock->pNext_ = nullptr;
	return (reinterpret_cast<char*>(pBlock) + sizeof(MemoryBlockHead));
}

//ͬ��ҲҪ����ƫ�������ж��Ƿ����ڴ����
void MemoryManger::deAllocMemory(void* p)
{
	char* q = reinterpret_cast<char*>(p) - sizeof(MemoryBlockHead);
	MemoryBlockHead* pBlock = reinterpret_cast<MemoryBlockHead*>(q);
	if (pBlock->isInMemoryPool_ == true)
	{
		//��ǰ�ڴ��֪��������һ���ڴ��
		pBlock->ownerMemoryPool_->deAllocMemory(p);
	}
	else
	{
		free(pBlock);
	}
}

MemoryManger::~MemoryManger()
{

}
