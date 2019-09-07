
#include <assert.h>
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread() :loop_(nullptr), \
thread_(std::bind(&EventLoopThread::threadFunc, this)),
mutex_(), cond_(), existing_(false)
{

}

//for call loop.loop()
//创建事件循环对象，并开启事件循环
void EventLoopThread::threadFunc()
{
	EventLoop loop;
	//根据任务执行下面循环操作

	{
		std::lock_guard<std::mutex> guard(mutex_);
		loop_ = &loop; //多个线程可能同时访问loop_
		cond_.notify_one(); //如果一个线程调用startLoop，但是loop并没有创建的情况
	}

	loop.loop();  /*开启事件循环*/
	assert(existing_);
	loop_ = nullptr;
}

EventLoop* EventLoopThread::getLoopOfCurrentThread()
{
	assert(thread_.joinable());  //可以判断线程是否存活

	{
		std::unique_lock<std::mutex> lk(mutex_); //能灵活加锁解锁
		cond_.wait(lk, [this] {return this->loop_ != nullptr; });
	}

	return loop_;
}

//析构的时候自动退出，并且回收线程
EventLoopThread::~EventLoopThread()
{
	existing_ = true;
	while (loop_ != nullptr)
	{
		loop_->quit();
		thread_.join();
	}
}