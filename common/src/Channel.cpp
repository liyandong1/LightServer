
#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

#include <assert.h>
#include <sys/epoll.h>

//����Ǿ���Ŀɶ���д�¼�
const int Channel::noneEvent = 0;
const int Channel::readEvent = EPOLLIN | EPOLLPRI | EPOLLET;
const int Channel::writeEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int argFd) :loop_(loop), fd_(argFd),
         monitorEvents_(0), happendEvents_(0), index_(-1)
{

}

//ͨ��EventLoop�Ѹı����Ϣע�ᵽepoll��
void Channel::fromLoopUpdateToEpoll()
{
	loop_->updateToepoll(this);  
}

int Channel::fd() const 
{
	return fd_;
}

int Channel::getHappendEvents() const //�����Ѿ���������
{
	return happendEvents_;
}

int Channel::getMonitorEvents() const
{
	return monitorEvents_;
}

int Channel::setEvents(int events) //����Ҫ��ص��¼�
{
	monitorEvents_ = events;
}

int Channel::setHappendEvents(int events)
{
	happendEvents_ = events;
}

//ʹ�ܻ����Ƴ���Ҫ�������¼�,��������ᴫ�͵�epoll��
void Channel::enableReading()
{
	monitorEvents_ |= readEvent;
	fromLoopUpdateToEpoll();
}

void Channel::enableWriting()
{
	monitorEvents_ |= writeEvent;
	fromLoopUpdateToEpoll();
}

void Channel::disableWriting()
{
	monitorEvents_ &= ~writeEvent;
	fromLoopUpdateToEpoll();
}

void Channel::disableAll()
{
	monitorEvents_ = noneEvent;
	fromLoopUpdateToEpoll();
}


//��epoll�õ�
int Channel::index() const
{
	return index_;
}

void Channel::setIndex(int index)
{
	index_ = index;
}

EventLoop* Channel::ownerLoop()
{
	return loop_;
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
	tie_ = obj;
}

//�¼��ַ���
void Channel::handleHappendEvents()
{
	if (happendEvents_ & EPOLLERR)
	{
		if (errorCallback_);
			errorCallback_();
	}
	if ((happendEvents_ & (EPOLLHUP)) && !(happendEvents_ & EPOLLIN))
	{
		LOG_ERROR << "Channel::handleEvent EPOLLHUP ";
		if (closeCallback_);
			closeCallback_();
	}
	if (happendEvents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (readCallback_)
			readCallback_();
	}
	if (happendEvents_ & EPOLLOUT)
	{
		if (writeCallback_);
		writeCallback_();
	}
}

void Channel::setReadCallback(const EventCallback& cb)
{
	readCallback_ = cb;
}

void Channel::setWriteCallback(const EventCallback& cb)
{
	writeCallback_ = cb;
}

void Channel::setErrorCallback(const EventCallback& cb)
{
	errorCallback_ = cb;
}

void Channel::setCloseCallback(const EventCallback& cb)
{
	closeCallback_ = cb;
}

void Channel::remove()
{
	loop_->removeChannel(this);
}

void Channel::removeChannel(Channel* channel)
{
	loop_->removeChannel(channel);
}

Channel::~Channel()
{

}
