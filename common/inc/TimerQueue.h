
#ifndef _TIMERQUEUE_H_
#define _TIMERQUEUE_H_

#include <set>
#include <atomic>
#include <memory>
#include <vector>
#include <functional>
#include "noncopyable.h"
#include "TimerNode.h"

class Channel;
class EventLoop;

struct TimerCmp
{
	bool operator()(TimerNode* a, TimerNode* b) const
	{
		return a->getExpTime() < b->getExpTime();
	}
};

//定时器队列,用红黑树实现
class TimerQueue : public noncopyable
{
private:
	EventLoop* ownerLoop_;
	const int timerfd_; //真正的定时器
	std::unique_ptr<Channel> timerfdChannel_; //和fd匹配的Channel，用于注册到epoll
	std::atomic<bool> callingExpireTimerEvents_; //是否正在处理超时事件

	std::vector<TimeCallback> expireEvent_;
	std::multiset<TimerNode*, TimerCmp> timers_;
public:
	explicit TimerQueue(EventLoop* loop);
	void addTimer(size_t timeout, const TimeCallback& cb);//这个函数可能跨线程调用

	void TimerQueuehandleTimerEvent(); //事件到了，处理到期事件
	void addTimerInLoop(TimerNode* timer);

	//for loop
	std::vector<TimeCallback> getExpired();//获取到期的事件
	~TimerQueue();
};

#endif
