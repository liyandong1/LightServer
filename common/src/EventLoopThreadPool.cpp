#include <assert.h>
#include "Logging.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base, size_t threadsNum) :
	baseLoop_(base), started_(false), threadsNum(threadsNum), next_(0)
{
	if (threadsNum <= 0) {
		LOG_ERROR << "Thread Num <= 0";
		abort();
	}
}

//创建线程并且开始
void EventLoopThreadPool::start()
{
	baseLoop_->assertInLoopThread();
	started_ = true;
	for (size_t i = 0; i < threadsNum; ++i) {
		EventLoopThread* t(new EventLoopThread());
		loopThreads_.push_back(std::shared_ptr<EventLoopThread>(t));
		loops_.push_back(t->getLoopOfCurrentThread());
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseLoop_->assertInLoopThread();
	assert(started_);
	EventLoop* loop = baseLoop_;
	if (!loops_.empty()) {
		loop = loops_[next_];
		next_ = (next_ + 1) % threadsNum;
	}
	return loop;
}

EventLoopThreadPool::~EventLoopThreadPool()
{
	LOG << "~EventLoopThreadPool()";
}