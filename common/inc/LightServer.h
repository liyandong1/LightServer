
#ifndef _LIGHTSERVER_H_
#define _LIGHTSERVER_H_

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <time.h>
#include <functional>
#include "CallBacks.h"
#include "noncopyable.h"

using namespace std::placeholders;

class Channel;
class Socket;
class EventLoop;
struct InetAddress;
class ClientConnection; //���ÿһ���ͻ��˵�����
class EventLoopThreadPool;

class LightServer : public noncopyable
{
private:
	EventLoop* loop_;
	std::unique_ptr<Socket> acceptSocket_;
	int listenFd;
	int nextCondId_;
	std::unique_ptr<Channel> acceptChannel_;
	bool listening_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;//ӵ��һ���̳߳�

	//��ͻ��˵���Ϣ�úú�дһ��
	std::vector<std::shared_ptr<Channel>> ClientChannels_;
	//�����ù�ϣ���Ż�
	std::map<std::string, std::shared_ptr<ClientConnection>> ClientConnectionMap_;

	//�ص�����
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
	
	time_t oldTime_; //�����������

	void handleRead();
	void onNewConnection(int fd, const InetAddress& peerAddr);

	//��Щ�ӿڻ�Ҫ�Ż�
	void removeConnection(const std::shared_ptr<ClientConnection>& conn);
	void removeConnectionInLoop(const std::shared_ptr<ClientConnection>& conn);
	
	//���ͻ�������
	void checkEveryClientAlive();
public:
	LightServer(EventLoop* loop, int listenPort, int threadNum);
	void listen();
	bool listening() const;

	//���ûص�
	void setConnectionCallback(const ConnectionCallback& cb);
	void setMessageCallback(const MessageCallback& cb);
	void setCloseCallback(const CloseCallback& cb);
	~LightServer();
};

#endif
