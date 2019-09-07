#ifndef _EPOLL_H_
#define _EPOLL_H_


#include <vector>
#include <unordered_map>
#include <sys/epoll.h>

class Channel;
class EventLoop;

class Epoll
{
private:
	int epollFd_;
	EventLoop* ownerLoop_;
	std::vector<::epoll_event> events_; //活跃事件

	int currentActiveEventSize;

	std::unordered_map<int, Channel*> listenChannels_;//注册到epoll上的事件
	
	//封装epoll_ctl
	void epollAdd(Channel* channel);
	void epollMod(Channel* channel);
	void epollDel(Channel* channel);
public:
	Epoll(EventLoop* loop);

	int getEpollFd() const;
	void updateChannel(Channel* channel);//用于修改注册到channel上的事件
	void removeChannel(Channel* channel);//删除注册到epoll上的事件

	void getActiveEvents();//获取活跃事件
	void getActiveEvents(std::vector<Channel*>& ret);

	void assertLoopInThread();

	~Epoll();
};

#endif
