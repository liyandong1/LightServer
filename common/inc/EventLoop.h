
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
	const std::thread::id threadId_;   //�¼�ѭ���������߳�id
	std::atomic<bool> looping_;
	std::atomic<bool> quit_;

	std::unique_ptr<Epoll> epoller_;  //epoll���
	std::vector<Channel*> activeChannels_; //��epoll�л�ȡ�Ļ�Ծ�¼�

	std::unique_ptr<TimerQueue> timerQueue_;//��ʱ�����
	std::vector<std::function<void()>> timerEvent_;

	std::mutex m_mutex;
	std::vector<std::function<void()>> otherThreadCalls;

	int wakeupFd_; //�̻߳���
	std::unique_ptr<Channel> wakeupFdChannel_;
	
	std::function<void()> heartCheckFunction_;

	void abortThread();  //��������ֹ�߳�
public:
	EventLoop();
	~EventLoop();

	bool isInLoopThread() const;
	void assertInLoopThread(); //����Ƿ��ڵ�ǰ�߳������¼�ѭ��

	void loop();
	void quit();

	static EventLoop* getEventLoopOfCurrentThread();//�������ڵ�ǰ�̵߳��¼�ѭ������
	void updateToepoll(Channel* channel);//����channel�е��¼����õ�eoll��ȥ

	void runAfter(size_t timeout, const std::function<void()>& cb);//���೤ʱ������
																   //����֧�ֿ��̵߳��õĺ���
	void runInLoop(const std::function<void()>& cb);
	void queueInLoop(const std::function<void()>& cb);

	//�̻߳���
	void wakeup();
	void handleEventFd();
	void processOthreThreadCalls();
	void removeChannel(Channel* channel);
	
	void setHeartCheckFunction(const std::function<void()>& cb);
};

#endif
