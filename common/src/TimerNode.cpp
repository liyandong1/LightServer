
#include "TimerNode.h"

#include <sys/time.h>


//for TimerNode

TimerNode::TimerNode(int timeout, TimeCallback cb) :
	relativeTime_(timeout), callback_(cb)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	//计算成ms
	expiredTime_ = ((((now.tv_sec + timeout) % 10000) * 1000) + (now.tv_usec / 1000));
}

void TimerNode::update(int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	expiredTime_ = (((now.tv_sec + timeout) * 1000) + (now.tv_usec / 1000));
	relativeTime_ = timeout;
}

//当前节点是否有效
bool TimerNode::isVaild()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
	if (temp < expiredTime_)
		return true;
	else
		return false;
}

size_t TimerNode::getExpTime() const
{
	return expiredTime_;
}

size_t TimerNode::getRelativeTime() const
{
	return relativeTime_;
}

TimeCallback TimerNode::getCallback() const
{
	return callback_;
}

bool TimerNode::operator<(const TimerNode& tmNode)
{
	return getExpTime() < tmNode.getExpTime();
}

TimerNode::~TimerNode()
{

}