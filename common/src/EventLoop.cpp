
#include "EventLoop.h"
#include "Logging.h"
#include "Channel.h"
#include "Epoll.h"
#include "TimerQueue.h"

#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h> 

__thread EventLoop* loopForThisThread = nullptr;

//创建非阻塞的wakeupfd
int createEventfd()
{
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		LOG_ERROR << "createEventfd error ";
		abort();
	}
	return evtfd;
}


EventLoop::EventLoop() :threadId_(std::this_thread::get_id()), looping_(false), \
quit_(false), epoller_(new Epoll(this)), timerQueue_(new TimerQueue(this)),
wakeupFd_(createEventfd()), wakeupFdChannel_(new Channel(this, wakeupFd_))
{
	LOG << "Event Loop created " << this << " in thread "<< threadId_;
	if (loopForThisThread)
	{
		LOG_ERROR << "Another EventLoop" << loopForThisThread << "exists in this thread";
	}
	else   //记录当前线程所拥有的事件循环对象
	{
		loopForThisThread = this;

		//注册eventfd到epoll
		assert(wakeupFd_ > 0);
		wakeupFdChannel_->setEvents(EPOLLIN | EPOLLET);  //ET
		wakeupFdChannel_->setReadCallback(std::bind(&EventLoop::handleEventFd, this));
		epoller_->updateChannel(wakeupFdChannel_.get());
	}
}

bool EventLoop::isInLoopThread() const
{
	return std::this_thread::get_id() == threadId_;
}

void EventLoop::abortThread()
{
	LOG_FATAL << "EventLoop::abortThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << std::this_thread::get_id();
}

//循环没有运行在当前线程，就应该abort()
void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread())
	{
		abortThread();
	}
}

//返回属于当前线程的事件循环对象
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return loopForThisThread;
}


void EventLoop::loop()
{
	assert(!looping_);//确保开始前没有在循环
	assertInLoopThread();//保证当前事件循环在当前线程被调用
	looping_ = true;
	quit_ = false;

	while (!quit_)
	{
		//处理epoll上的活跃事件
		activeChannels_.clear();
		epoller_->getActiveEvents(activeChannels_);
		for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
		{
			(*it)->handleHappendEvents();
		}

		//处理定时器事件
		timerEvent_.clear();
		timerEvent_ = timerQueue_->getExpired();
		for (size_t i = 0; i < timerEvent_.size(); ++i) {
			timerEvent_[i]();
		}
		//处理其他线程调用事件
		processOthreThreadCalls();
		
		//如果当前事件循环需要心跳检测，为服务端循环设计
		if(heartCheckFunction_)
			heartCheckFunction_();
	}
	LOG << "EventLoop " << this << " stop looping ";
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::updateToepoll(Channel* channel)
{
	//真的设置到epoll上去
	assert(channel->ownerLoop() == this);
	assertInLoopThread();

	epoller_->updateChannel(channel);
}

void EventLoop::runAfter(size_t timeout, const std::function<void()>& cb)
{
	timerQueue_->addTimer(timeout, cb);
}

//支持跨线程调用
void EventLoop::runInLoop(const std::function<void()>& cb)
{
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(cb);
	}
}

//这里要加锁，可能多个线程都放东西进来,临界区代码
void EventLoop::queueInLoop(const std::function<void()>& cb)
{
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		otherThreadCalls.push_back(cb);
	}
	if (!isInLoopThread())
		wakeup();  //唤醒该事件所属的线程
}

//写到eventfd里面，唤醒线程
void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << "bytes instead of 8";
	}
}

//处理eventfd可读事件,这个事件是被epoll发现的，重新注册eventfd可读事件
void EventLoop::handleEventFd()
{
	uint64_t one = 1;
	ssize_t n = read(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << "bytes instead of 8";
	}
	wakeupFdChannel_->setEvents(EPOLLIN | EPOLLET);//重新注册可读事件
	epoller_->updateChannel(wakeupFdChannel_.get());
}

//要处理临接的调用队列，需要进行加锁
void EventLoop::processOthreThreadCalls()
{
	std::vector<std::function<void()>> otherCalls;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		otherCalls.swap(otherThreadCalls);
	}

	for (size_t i = 0; i < otherCalls.size(); i++) {
		otherCalls[i]();
	}

	//std::cout << "processOthreThreadCalls()" << std::endl;
}

void EventLoop::removeChannel(Channel* channel)
{
	epoller_->removeChannel(channel);
}

void EventLoop::setHeartCheckFunction(const std::function<void()>& cb)
{
	heartCheckFunction_ = cb;
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	loopForThisThread = nullptr;
}
