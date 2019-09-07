
#include "Epoll.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

const int EVENTSUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll(EventLoop* loop) :epollFd_(epoll_create(EPOLL_CLOEXEC)), ownerLoop_(loop), \
events_(EVENTSUM)
{
	currentActiveEventSize = 0;
	assert(epollFd_ > 0);
}

void Epoll::epollAdd(Channel* channel)
{
	int fd = channel->fd();
	struct epoll_event event;
	event.data.ptr = reinterpret_cast<void*>(channel);
	event.events = channel->getMonitorEvents();
	if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
		LOG_ERROR << "epollAdd error... ";
		perror("epollAdd error...");
	}
}

void Epoll::epollMod(Channel* channel)
{
	int fd = channel->fd();
	struct epoll_event event;
	event.data.ptr = reinterpret_cast<void*>(channel);
	event.events = channel->getMonitorEvents();
	if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
		LOG_ERROR << "epollMod error... ";
		perror("epollMod error...");
	}
}

void Epoll::epollDel(Channel* channel)
{
	int fd = channel->fd();
	if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
		LOG_ERROR << "epollDel error... ";
		perror("epollDel error...");
	}
}

int Epoll::getEpollFd() const
{
	return epollFd_;
}

void Epoll::assertLoopInThread()
{
	ownerLoop_->assertInLoopThread();
}

//index就是状态机，根据index判断是修改还是添加
// < 0未添加   ==1已添加  注意状态变化
void Epoll::updateChannel(Channel* channel)//用于修改注册到channel上的事件
{
	assertLoopInThread();
	if (channel->index() < 0)//a new channel
	{
		int fd = channel->fd();
		epollAdd(channel);
		listenChannels_.insert(std::make_pair(fd, channel));
		channel->setIndex(1);
	}
	else//update existing one
	{
		int fd = channel->fd();
		epollMod(channel);
		listenChannels_[fd] = channel;
	}
}

void Epoll::removeChannel(Channel* channel)//删除注册到epoll上的事件
{
	assertLoopInThread();
	int fd = channel->fd();
	epollDel(channel);
	listenChannels_.erase(fd);
	channel->setIndex(-1); // -1
}


void Epoll::getActiveEvents() //获取活跃事件
{
	int eventCount = ::epoll_wait(epollFd_, &*events_.begin(),
		static_cast<int>(events_.size()), EPOLLWAIT_TIME);
	if (eventCount > 0)
	{
		currentActiveEventSize = eventCount;
	}
}

//用户调用取得活跃事件
void Epoll::getActiveEvents(std::vector<Channel*>& ret)
{
	getActiveEvents();//先通过epoll获取活跃事件

	for (size_t i = 0; i < currentActiveEventSize; ++i)
	{
		Channel* channel(reinterpret_cast<Channel*>(events_[i].data.ptr));
		//这个是结构体里面被epoll设置的

		channel->setHappendEvents(events_[i].events);


		channel->setEvents(0); //取走事件后应该将监控事件清空
		ret.push_back(channel);
	}

	currentActiveEventSize = 0;
}

Epoll::~Epoll()
{
	close(epollFd_);
}
