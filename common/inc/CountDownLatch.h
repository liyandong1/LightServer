
#ifndef _COUNTDOWNLATCH_H_
#define _COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

//CountDownLatch����Ҫ������ȷ��thread�д���ȥ��
//func�������غ�����start�ŷ���

class CountDownLatch : public noncopyable
{
private:
	std::mutex mutex_;
	std::condition_variable condition_;
	int count_;
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();
	~CountDownLatch();
};

#endif