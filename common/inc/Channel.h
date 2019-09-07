
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <memory>
#include <functional>

class EventLoop;


//用于epoll的回调机制,分发和文件描述符相关事件
//Channel并不关闭fd
class Channel
{
public:
	typedef std::function<void()> EventCallback;
private:
	EventLoop* loop_;  //所属事件循环
	int fd_;
	int index_;
	int monitorEvents_; //需要监视的事件
	int happendEvents_; //当前已经发生的事件

	std::weak_ptr<void> tie_;

	EventCallback readCallback_; //可读事件的回调
	EventCallback writeCallback_; //可写事件的回调
	EventCallback errorCallback_;//错误事件的回调
	EventCallback closeCallback_;

	//定义具体的事件常量，方便使用
	static const int noneEvent;
	static const int readEvent;
	static const int writeEvent;

	void fromLoopUpdateToEpoll();
public:
	Channel(EventLoop* loop, int argFd);
	int fd() const; //返回监视的文件描述符
	int getHappendEvents() const; //返回已经发生的事件
	int getMonitorEvents() const; //返回要监控的事件
	int setEvents(int events); //设置要监控的事件
	int setHappendEvents(int events);//被设置已经发生的事件

	//使能或者移除需要监听的事件
	void enableReading();
	void enableWriting();
	void disableWriting();
	void disableAll();

	//设置具体的回调函数
	void setReadCallback(const EventCallback& cb);
	void setWriteCallback(const EventCallback& cb);
	void setErrorCallback(const EventCallback& cb);
	void setCloseCallback(const EventCallback& cb);

	//for epoll
	int index() const;
	void setIndex(int index);

	EventLoop* ownerLoop();
	void tie(const std::shared_ptr<void>& obj);
	void remove();
	void removeChannel(Channel* channel);

	//核心操作，分发事件的函数
	void handleHappendEvents();

	~Channel();
};

#endif
