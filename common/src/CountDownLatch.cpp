
#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
	:mutex_(), count_(count)
{

}

void CountDownLatch::wait()
{
	std::unique_lock<std::mutex> lk(mutex_);
	condition_.wait(lk, [this] {return this->count_ <= 0; });
}

void CountDownLatch::countDown()
{
	std::unique_lock<std::mutex> lk(mutex_);
	--count_;
	if (count_ == 0)
		condition_.notify_all();
}

CountDownLatch::~CountDownLatch()
{

}
