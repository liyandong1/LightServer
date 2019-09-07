
#ifndef _EVENTLOOP_H_
#define _EVENTlOOP_H_

#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include "noncopyable.h"

class Epoll;
class Channel;
class TimerQueue;

class EventLoop
{
private:
	const std::thread::id threadId_;   //事件循环所属的线程id
	std::atomic<bool> looping_;
	std::atomic<bool> quit_;

	std::unique_ptr<Epoll> epoller_;  //epoll组件
	std::vector<Channel*> activeChannels_; //从epoll中获取的活跃事件

	std::unique_ptr<TimerQueue> timerQueue_;//定时器组件
	std::vector<std::function<void()>> timerEvent_;

	std::mutex m_mutex;
	std::vector<std::function<void()>> otherThreadCalls;

	int wakeupFd_; //线程唤醒
	std::unique_ptr<Channel> wakeupFdChannel_;
	
	std::function<void()> heartCheckFunction_;

	void abortThread();  //非正常终止线程
public:
	EventLoop();
	~EventLoop();

	bool isInLoopThread() const;
	void assertInLoopThread(); //检查是否在当前线程运行事件循环

	void loop();
	void quit();

	static EventLoop* getEventLoopOfCurrentThread();//返回属于当前线程的事件循环对象
	void updateToepoll(Channel* channel);//根据channel中的事件设置到eoll上去

	void runAfter(size_t timeout, const std::function<void()>& cb);//过多长时间后调用
																   //用于支持跨线程调用的函数
	void runInLoop(const std::function<void()>& cb);
	void queueInLoop(const std::function<void()>& cb);

	//线程唤醒
	void wakeup();
	void handleEventFd();
	void processOthreThreadCalls();
	void removeChannel(Channel* channel);
	
	void setHeartCheckFunction(const std::function<void()>& cb);
};

#endif
