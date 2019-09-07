#ifndef _EVENTLOOPTHREADPOOL_H_
#define _EVENTLOOPTHREADPOOL_H_

#include <memory>
#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool :public noncopyable
{
private:
	EventLoop* baseLoop_;  //其实就是主线程循环
	bool started_;
	int threadsNum;
	int next_;
	std::vector<EventLoop*> loops_;//loopThreads_的生命周期存在，他就存在
	std::vector<std::shared_ptr<EventLoopThread>> loopThreads_;

public:
	EventLoopThreadPool(EventLoop* base, size_t threadsNum);
	void start();
	EventLoop* getNextLoop();

	~EventLoopThreadPool();
};

#endif