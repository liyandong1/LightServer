#ifndef _MAMORYMANGER_H_
#define _MEMORYmANGER_H_

#define MAX_MEMORY_SIZE 1024

#include <mutex>

class MemoryPool;

//�ڴ��ͷ����Ϣ
struct MemoryBlockHead
{
public:
	MemoryPool* ownerMemoryPool_;//�����ڴ��
	MemoryBlockHead* pNext_;//��һ�����̿�
	int blockId_;
	int blockRef_;
	bool isInMemoryPool_;//�Ƿ������ڴ�����ڴ�
private:
	char c1;
	char c2;
	char c3;
};

//const int a = sizeof(MemoryBlockHead);

class MemoryPool
{
protected:
	size_t currentMemBlockSize_;  //ÿһ��Ĵ�С
	size_t blockCount_;

	char* pBuf_;//ϵͳ������ڴ��׵�ַ
	MemoryBlockHead* pHead_; //��ջ�����ڴ�飬ָ��ǰ����������ڴ��

	std::mutex m_mutex; //���߳�

	//��ʼ���ڴ��
	void initMemoryPool();
public:
	void* alloMemory(size_t size);
	void deAllocMemory(void* p);
	MemoryPool();
	~MemoryPool();
};

template <size_t memBlockSize, size_t count>
class SpecificMemoryPool:public MemoryPool
{
public:
	SpecificMemoryPool()
	{
		const size_t n = sizeof(void*);
		currentMemBlockSize_ = ((memBlockSize / n) * n) + ((memBlockSize% n == 0) ? 0 : n);
		blockCount_ = count;
	}

	~SpecificMemoryPool() {}
};

class MemoryManger
{
private:
	SpecificMemoryPool<64, 10> mem64_;
	SpecificMemoryPool<128, 10> mem128_;
	SpecificMemoryPool<256, 10> mem256_;
	SpecificMemoryPool<512, 10> mem512_;
	SpecificMemoryPool<1024, 10> mem1024_;

	MemoryPool* memoryPoolMap[MAX_MEMORY_SIZE + 1];//�ڴ��ӳ������

	MemoryManger(const MemoryManger&) = delete;
	MemoryManger& operator=(const MemoryManger&) = delete;
	MemoryManger();

	//Ϊ����Χ���ڴ�齨��ӳ���ϵ
	void createMapRelatition(size_t begin, size_t end, MemoryPool* pMem);
public:
	//��̬�������д��
	static MemoryManger& getInstance();
	//�����ڴ�
	void* allocMemory(size_t size);
	void deAllocMemory(void* p);
	~MemoryManger();
};



#endif
