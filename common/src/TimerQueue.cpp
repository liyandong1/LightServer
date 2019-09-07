
#include <utility>
#include <sys/time.h>
#include <assert.h>
#include <strings.h>
#include <sys/timerfd.h>
#include "TimerQueue.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"


TimerQueue::TimerQueue(EventLoop* loop) :ownerLoop_(loop), \
timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
timerfdChannel_(new Channel(loop, timerfd_)), callingExpireTimerEvents_(false)
{
	if (timerfd_ < 0) {
		LOG << "TimerQueue timerfd_ error";
		assert(!(timerfd_ < 0));
	}
	struct itimerspec howlong;
	::bzero(&howlong, sizeof(howlong));
	howlong.it_value.tv_sec = 1;
	::timerfd_settime(timerfd_, 0, &howlong, NULL);
	//Ϊtimerfd���ûص�
	timerfdChannel_->setReadCallback(std::bind(&TimerQueue::TimerQueuehandleTimerEvent, this));
	//ע�ᵽEpoll��
	timerfdChannel_->enableReading();
}

//Ҫ֧�ֿ��̵߳���
void TimerQueue::addTimer(size_t timeout, const TimeCallback& cb)
{
	TimerNode* newTimer = new TimerNode(timeout, cb);
	//timers_.insert(newTimer);
	ownerLoop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, newTimer));
}

void TimerQueue::addTimerInLoop(TimerNode* timer)
{
	ownerLoop_->assertInLoopThread();
	timers_.insert(timer);
}

void TimerQueue::TimerQueuehandleTimerEvent() //�¼����ˣ��������¼�
{
	callingExpireTimerEvents_ = true;

	TimerNode curNode(0, nullptr); //��ǰʱ��ڵ�
	//std::cout << "Current time = " << curNode.getExpTime() << std::endl;

	auto it = timers_.lower_bound(&curNode);

	for (auto p = timers_.begin(); p != it; ++p) {
		//std::cout << "time = " << (*p)->getExpTime() << std::endl;
		expireEvent_.push_back((*p)->getCallback());
		delete (*p);
	}

	timers_.erase(timers_.begin(), it);
	callingExpireTimerEvents_ = false;

	//std::cout << "TimerQueuehandleTimerEvent()" << std::endl;
	//�ٴ�ע�ᵽEpoll��
	timerfdChannel_->enableReading();
}


//for loop
std::vector<TimeCallback> TimerQueue::getExpired()//��ȡ���ڵ��¼�
{
	std::vector<TimeCallback> ret(expireEvent_);
	expireEvent_.clear();
	return ret;
}

TimerQueue::~TimerQueue()
{
	for (auto it = timers_.begin(); it != timers_.end(); ++it) {
		delete (*it);
	}
	timers_.clear();
}
