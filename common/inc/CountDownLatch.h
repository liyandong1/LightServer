
#ifndef _COUNTDOWNLATCH_H_
#define _COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

//CountDownLatch的主要作用是确保thread中传进去的
//func真正返回后，外层的start才返回

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