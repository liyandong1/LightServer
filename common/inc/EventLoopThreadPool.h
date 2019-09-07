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
	EventLoop* baseLoop_;  //��ʵ�������߳�ѭ��
	bool started_;
	int threadsNum;
	int next_;
	std::vector<EventLoop*> loops_;//loopThreads_���������ڴ��ڣ����ʹ���
	std::vector<std::shared_ptr<EventLoopThread>> loopThreads_;

public:
	EventLoopThreadPool(EventLoop* base, size_t threadsNum);
	void start();
	EventLoop* getNextLoop();

	~EventLoopThreadPool();
};

#endif