
#ifndef _TIMERNODE_H_
#define _TIMERNODE_H_

#include "noncopyable.h"
#include <functional>

typedef std::function<void()> TimeCallback;

//时间节点，时间到了去做具体的事情，相当于Channel
class TimerNode
{
private:
	size_t expiredTime_;  //过期时间，绝对时间（ms）
	size_t relativeTime_; //相对时间
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