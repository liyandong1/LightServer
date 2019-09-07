
#ifndef _CLIENTCONNECTION_H_
#define _CLIENTCONNECTION_H_

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <time.h>
#include "Buffer.h"
#include "noncopyable.h"
#include "ObjectPool.h"
#include "CallBacks.h"

class Socket;
class Channel;
class EventLoop;

#define HEART_TIME 10000   //10000ms    10s

class ClientConnection : public noncopyable, public ObjectPoolBase<ClientConnection, 100>,
	                     public std::enable_shared_from_this<ClientConnection>
{
private:
	EventLoop* loop_;
	std::unique_ptr<Channel> clientChannel_;
	std::unique_ptr<Socket> clientSocket_;
	std::string name_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;

	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
	ConnectionCallback connectionCallback_;
	
	time_t heartTime_;  //心跳检测, 存放上一次心跳的时间

	//每个客户端都有处理这些事情的方法
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
public:
	ClientConnection(EventLoop* loop, const std::string& name, int connfd);
	void setMessageCallback(const MessageCallback& cb);
	void setCloseCallback(const CloseCallback& cb);
	void setConnectionCallback(const ConnectionCallback& cb);
	EventLoop* getLoop();
	void connectEstablished();
	void connectDestory();
	std::string name()
	{
		return name_;
	}
	void sendMessage(const char* msg, size_t len);
	void resetHeartTime(); //重置心跳计时
	bool checkHeart(time_t dt);
	~ClientConnection();
};

#endif
