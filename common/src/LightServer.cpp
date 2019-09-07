
#include <unistd.h>
#include <string.h>
#include "Socket.h"
#include "Channel.h"
#include "Logging.h"
#include "TimeStamp.h"
#include "LightServer.h"
#include "EventLoop.h"
#include "ClientConnection.h"
#include "EventLoopThreadPool.h"

#include <iostream>
using namespace std;

LightServer::LightServer(EventLoop* loop, int listenPort, int threadNum):loop_(loop),
	acceptSocket_(new Socket(listenPort)), listenFd(acceptSocket_->fd()), nextCondId_(0),
	acceptChannel_(new Channel(loop, acceptSocket_->fd())), listening_(false),
	threadPool_(new EventLoopThreadPool(loop, threadNum))
{
	//绑定ip
	acceptSocket_->bindAddress();//这儿可以设置参数
	acceptSocket_->setReuseAddr(true);
	acceptSocket_->setReusePort(true);
	//设置监听描述符回调
	acceptChannel_->setReadCallback(std::bind(&LightServer::handleRead, this));
	
	//服务端循环需要检测心跳
	loop_->setHeartCheckFunction(std::bind(&LightServer::checkEveryClientAlive, this));

	//debug
	cout << "ListenPort " << listenPort << std::endl;
	cout << "threadNum " << threadNum << std::endl;
	//cout << "IP " << m_config->getStr("IP") << std::endl;
	//启动线程池
	threadPool_->start();
}

void LightServer::listen()
{
	loop_->assertInLoopThread();
	listening_ = true;
	acceptSocket_->listen();
	acceptChannel_->enableReading();
}

bool LightServer::listening() const
{
	return listening_;
}

//当监听socket可读,也就是有新的客户端连接上来
void LightServer::handleRead()
{
	loop_->assertInLoopThread();
	//这个位置可以给线程分派任务了
	InetAddress peerAddr;
	int connfd = acceptSocket_->accept(&peerAddr);
	if (connfd >= 0)
	{
		onNewConnection(connfd, peerAddr);
	}
	else
	{
		::close(connfd);
	}
}


//好好梳理一下执行逻辑，这里容易崩溃
void LightServer::onNewConnection(int fd, const InetAddress& peerAddr)
{

	loop_->assertInLoopThread();
	//给新连接上来的客户分配id
	std::string connName = std::to_string(nextCondId_++);
	LOG << "LightServer::newConnection [" << connName
		<< "] from " << peerAddr.ip_ << " " << peerAddr.port_;
	//从线程池中选择线程
	EventLoop* ioLoop = threadPool_->getNextLoop();
	//创建连接对象
	std::shared_ptr<ClientConnection> conn(new ClientConnection(ioLoop, connName, fd));
	ClientConnectionMap_[connName] = conn;

	//给每个客户端设置回调函数
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(closeCallback_);
	conn->setConnectionCallback(connectionCallback_);
	//一定要放到io线程调用，跨线程调用序列
	//回来实现这个函数  (客户端被分给了工作线程，函数只能在工作线程被调用)
	//告诉工作线程，启用当前客户端
	ioLoop->runInLoop(std::bind(&ClientConnection::connectEstablished, conn));

}

//对每个客户端更改的操作，都应该放到客户端所属线程来完成调用
//请注意：：当前客户端所属线程
void LightServer::removeConnection(const ClientConnectionPtr& conn)
{
	loop_->runInLoop(std::bind(&LightServer::removeConnectionInLoop, this, conn));
}

void LightServer::removeConnectionInLoop(const ClientConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	LOG << "HttpServer::removeConnection [ " << conn->name() << " ] - connection ";
	ClientConnectionMap_.erase(conn->name());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&ClientConnection::connectDestory, conn));
}

void LightServer::checkEveryClientAlive()
{
	auto nowTime = Time::getNowInMillSec();
	auto dt = nowTime - oldTime_;//计算上一次的时间经過
	oldTime_ = nowTime;
	//不用每个客户端都传递事件，更加高效
	for(auto it = ClientConnectionMap_.begin(); it != ClientConnectionMap_.end();)
	{
		if(it->second->checkHeart(dt))//death
		{
			//移除   connectDestory();   ClientConnection
			it->second->getLoop()->runInLoop(std::bind(&ClientConnection::connectDestory, it->second));
			ClientConnectionMap_.erase(it++);
		}
		else
		{
			it ++;
		}
	}
}

void LightServer::setConnectionCallback(const ConnectionCallback& cb)
{
	connectionCallback_ = cb;
}

void LightServer::setMessageCallback(const MessageCallback& cb)
{
	messageCallback_ = cb;
}

void LightServer::setCloseCallback(const CloseCallback& cb)
{
	closeCallback_ = cb;
}


//还要关闭客户端
LightServer::~LightServer()
{
	loop_->assertInLoopThread();
	LOG << "TcpServer::~TcpServer";
	for (auto& item : ClientConnectionMap_)
	{
		ClientConnectionPtr conn(item.second);
		item.second.reset();
		//让每个tcp客户端的线程自己关闭
		conn->getLoop()->runInLoop(
			std::bind(&ClientConnection::connectDestory, conn));
	}
	::close(listenFd);
}
