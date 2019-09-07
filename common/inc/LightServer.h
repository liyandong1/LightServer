
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
class ClientConnection; //针对每一个客户端的连接
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
	std::unique_ptr<EventLoopThreadPool> threadPool_;//拥有一个线程池

	//存客户端的信息得好好写一下
	std::vector<std::shared_ptr<Channel>> ClientChannels_;
	//可以用哈希表优化
	std::map<std::string, std::shared_ptr<ClientConnection>> ClientConnectionMap_;

	//回调函数
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
	
	time_t oldTime_; //用于心跳检测

	void handleRead();
	void onNewConnection(int fd, const InetAddress& peerAddr);

	//这些接口还要优化
	void removeConnection(const std::shared_ptr<ClientConnection>& conn);
	void removeConnectionInLoop(const std::shared_ptr<ClientConnection>& conn);
	
	//检测客户端心跳
	void checkEveryClientAlive();
public:
	LightServer(EventLoop* loop, int listenPort, int threadNum);
	void listen();
	bool listening() const;

	//设置回调
	void setConnectionCallback(const ConnectionCallback& cb);
	void setMessageCallback(const MessageCallback& cb);
	void setCloseCallback(const CloseCallback& cb);
	~LightServer();
};

#endif
