
#ifndef _TIMERNODE_H_
#define _TIMERNODE_H_

#include "noncopyable.h"
#include <functional>

typedef std::function<void()> TimeCallback;

//ʱ��ڵ㣬ʱ�䵽��ȥ����������飬�൱��Channel
class TimerNode
{
private:
	size_t expiredTime_;  //����ʱ�䣬����ʱ�䣨ms��
	size_t relativeTime_; //���ʱ��
	TimeCallback callback_;
public:
	TimerNode(int timeout, TimeCallback cb);
	void update(int timeout);
	bool isVaild();
	size_t getExpTime() const;
	size_t getRelativeTime() const;
	TimeCallback getCallback() const;
	bool operator<(const TimerNode& tmNode);
	~TimerNode();
};

#endif