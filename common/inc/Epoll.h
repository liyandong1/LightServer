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
	std::vector<::epoll_event> events_; //��Ծ�¼�

	int currentActiveEventSize;

	std::unordered_map<int, Channel*> listenChannels_;//ע�ᵽepoll�ϵ��¼�
	
	//��װepoll_ctl
	void epollAdd(Channel* channel);
	void epollMod(Channel* channel);
	void epollDel(Channel* channel);
public:
	Epoll(EventLoop* loop);

	int getEpollFd() const;
	void updateChannel(Channel* channel);//�����޸�ע�ᵽchannel�ϵ��¼�
	void removeChannel(Channel* channel);//ɾ��ע�ᵽepoll�ϵ��¼�

	void getActiveEvents();//��ȡ��Ծ�¼�
	void getActiveEvents(std::vector<Channel*>& ret);

	void assertLoopInThread();

	~Epoll();
};

#endif
