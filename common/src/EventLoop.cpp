
#include "EventLoop.h"
#include "Logging.h"
#include "Channel.h"
#include "Epoll.h"
#include "TimerQueue.h"

#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h> 

__thread EventLoop* loopForThisThread = nullptr;

//������������wakeupfd
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
	else   //��¼��ǰ�߳���ӵ�е��¼�ѭ������
	{
		loopForThisThread = this;

		//ע��eventfd��epoll
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

//ѭ��û�������ڵ�ǰ�̣߳���Ӧ��abort()
void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread())
	{
		abortThread();
	}
}

//�������ڵ�ǰ�̵߳��¼�ѭ������
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return loopForThisThread;
}


void EventLoop::loop()
{
	assert(!looping_);//ȷ����ʼǰû����ѭ��
	assertInLoopThread();//��֤��ǰ�¼�ѭ���ڵ�ǰ�̱߳�����
	looping_ = true;
	quit_ = false;

	while (!quit_)
	{
		//����epoll�ϵĻ�Ծ�¼�
		activeChannels_.clear();
		epoller_->getActiveEvents(activeChannels_);
		for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
		{
			(*it)->handleHappendEvents();
		}

		//����ʱ���¼�
		timerEvent_.clear();
		timerEvent_ = timerQueue_->getExpired();
		for (size_t i = 0; i < timerEvent_.size(); ++i) {
			timerEvent_[i]();
		}
		//���������̵߳����¼�
		processOthreThreadCalls();
		
		//�����ǰ�¼�ѭ����Ҫ������⣬Ϊ�����ѭ�����
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
	//������õ�epoll��ȥ
	assert(channel->ownerLoop() == this);
	assertInLoopThread();

	epoller_->updateChannel(channel);
}

void EventLoop::runAfter(size_t timeout, const std::function<void()>& cb)
{
	timerQueue_->addTimer(timeout, cb);
}

//֧�ֿ��̵߳���
void EventLoop::runInLoop(const std::function<void()>& cb)
{
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(cb);
	}
}

//����Ҫ���������ܶ���̶߳��Ŷ�������,�ٽ�������
void EventLoop::queueInLoop(const std::function<void()>& cb)
{
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		otherThreadCalls.push_back(cb);
	}
	if (!isInLoopThread())
		wakeup();  //���Ѹ��¼��������߳�
}

//д��eventfd���棬�����߳�
void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << "bytes instead of 8";
	}
}

//����eventfd�ɶ��¼�,����¼��Ǳ�epoll���ֵģ�����ע��eventfd�ɶ��¼�
void EventLoop::handleEventFd()
{
	uint64_t one = 1;
	ssize_t n = read(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << "bytes instead of 8";
	}
	wakeupFdChannel_->setEvents(EPOLLIN | EPOLLET);//����ע��ɶ��¼�
	epoller_->updateChannel(wakeupFdChannel_.get());
}

//Ҫ�����ٽӵĵ��ö��У���Ҫ���м���
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
