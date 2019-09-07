
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <memory>
#include <functional>

class EventLoop;


//����epoll�Ļص�����,�ַ����ļ�����������¼�
//Channel�����ر�fd
class Channel
{
public:
	typedef std::function<void()> EventCallback;
private:
	EventLoop* loop_;  //�����¼�ѭ��
	int fd_;
	int index_;
	int monitorEvents_; //��Ҫ���ӵ��¼�
	int happendEvents_; //��ǰ�Ѿ��������¼�

	std::weak_ptr<void> tie_;

	EventCallback readCallback_; //�ɶ��¼��Ļص�
	EventCallback writeCallback_; //��д�¼��Ļص�
	EventCallback errorCallback_;//�����¼��Ļص�
	EventCallback closeCallback_;

	//���������¼�����������ʹ��
	static const int noneEvent;
	static const int readEvent;
	static const int writeEvent;

	void fromLoopUpdateToEpoll();
public:
	Channel(EventLoop* loop, int argFd);
	int fd() const; //���ؼ��ӵ��ļ�������
	int getHappendEvents() const; //�����Ѿ��������¼�
	int getMonitorEvents() const; //����Ҫ��ص��¼�
	int setEvents(int events); //����Ҫ��ص��¼�
	int setHappendEvents(int events);//�������Ѿ��������¼�

	//ʹ�ܻ����Ƴ���Ҫ�������¼�
	void enableReading();
	void enableWriting();
	void disableWriting();
	void disableAll();

	//���þ���Ļص�����
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

	//���Ĳ������ַ��¼��ĺ���
	void handleHappendEvents();

	~Channel();
};

#endif
