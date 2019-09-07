
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

//��ʱ������,�ú����ʵ��
class TimerQueue : public noncopyable
{
private:
	EventLoop* ownerLoop_;
	const int timerfd_; //�����Ķ�ʱ��
	std::unique_ptr<Channel> timerfdChannel_; //��fdƥ���Channel������ע�ᵽepoll
	std::atomic<bool> callingExpireTimerEvents_; //�Ƿ����ڴ���ʱ�¼�

	std::vector<TimeCallback> expireEvent_;
	std::multiset<TimerNode*, TimerCmp> timers_;
public:
	explicit TimerQueue(EventLoop* loop);
	void addTimer(size_t timeout, const TimeCallback& cb);//����������ܿ��̵߳���

	void TimerQueuehandleTimerEvent(); //�¼����ˣ��������¼�
	void addTimerInLoop(TimerNode* timer);

	//for loop
	std::vector<TimeCallback> getExpired();//��ȡ���ڵ��¼�
	~TimerQueue();
};

#endif
