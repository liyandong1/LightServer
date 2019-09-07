
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

//index����״̬��������index�ж����޸Ļ������
// < 0δ���   ==1�����  ע��״̬�仯
void Epoll::updateChannel(Channel* channel)//�����޸�ע�ᵽchannel�ϵ��¼�
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

void Epoll::removeChannel(Channel* channel)//ɾ��ע�ᵽepoll�ϵ��¼�
{
	assertLoopInThread();
	int fd = channel->fd();
	epollDel(channel);
	listenChannels_.erase(fd);
	channel->setIndex(-1); // -1
}


void Epoll::getActiveEvents() //��ȡ��Ծ�¼�
{
	int eventCount = ::epoll_wait(epollFd_, &*events_.begin(),
		static_cast<int>(events_.size()), EPOLLWAIT_TIME);
	if (eventCount > 0)
	{
		currentActiveEventSize = eventCount;
	}
}

//�û�����ȡ�û�Ծ�¼�
void Epoll::getActiveEvents(std::vector<Channel*>& ret)
{
	getActiveEvents();//��ͨ��epoll��ȡ��Ծ�¼�

	for (size_t i = 0; i < currentActiveEventSize; ++i)
	{
		Channel* channel(reinterpret_cast<Channel*>(events_[i].data.ptr));
		//����ǽṹ�����汻epoll���õ�

		channel->setHappendEvents(events_[i].events);


		channel->setEvents(0); //ȡ���¼���Ӧ�ý�����¼����
		ret.push_back(channel);
	}

	currentActiveEventSize = 0;
}

Epoll::~Epoll()
{
	close(epollFd_);
}
