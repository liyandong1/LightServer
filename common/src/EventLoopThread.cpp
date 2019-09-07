
#include <assert.h>
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread() :loop_(nullptr), \
thread_(std::bind(&EventLoopThread::threadFunc, this)),
mutex_(), cond_(), existing_(false)
{

}

//for call loop.loop()
//�����¼�ѭ�����󣬲������¼�ѭ��
void EventLoopThread::threadFunc()
{
	EventLoop loop;
	//��������ִ������ѭ������

	{
		std::lock_guard<std::mutex> guard(mutex_);
		loop_ = &loop; //����߳̿���ͬʱ����loop_
		cond_.notify_one(); //���һ���̵߳���startLoop������loop��û�д��������
	}

	loop.loop();  /*�����¼�ѭ��*/
	assert(existing_);
	loop_ = nullptr;
}

EventLoop* EventLoopThread::getLoopOfCurrentThread()
{
	assert(thread_.joinable());  //�����ж��߳��Ƿ���

	{
		std::unique_lock<std::mutex> lk(mutex_); //������������
		cond_.wait(lk, [this] {return this->loop_ != nullptr; });
	}

	return loop_;
}

//������ʱ���Զ��˳������һ����߳�
EventLoopThread::~EventLoopThread()
{
	existing_ = true;
	while (loop_ != nullptr)
	{
		loop_->quit();
		thread_.join();
	}
}