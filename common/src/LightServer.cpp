
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
	//��ip
	acceptSocket_->bindAddress();//����������ò���
	acceptSocket_->setReuseAddr(true);
	acceptSocket_->setReusePort(true);
	//���ü����������ص�
	acceptChannel_->setReadCallback(std::bind(&LightServer::handleRead, this));
	
	//�����ѭ����Ҫ�������
	loop_->setHeartCheckFunction(std::bind(&LightServer::checkEveryClientAlive, this));

	//debug
	cout << "ListenPort " << listenPort << std::endl;
	cout << "threadNum " << threadNum << std::endl;
	//cout << "IP " << m_config->getStr("IP") << std::endl;
	//�����̳߳�
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

//������socket�ɶ�,Ҳ�������µĿͻ�����������
void LightServer::handleRead()
{
	loop_->assertInLoopThread();
	//���λ�ÿ��Ը��̷߳���������
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


//�ú�����һ��ִ���߼����������ױ���
void LightServer::onNewConnection(int fd, const InetAddress& peerAddr)
{

	loop_->assertInLoopThread();
	//�������������Ŀͻ�����id
	std::string connName = std::to_string(nextCondId_++);
	LOG << "LightServer::newConnection [" << connName
		<< "] from " << peerAddr.ip_ << " " << peerAddr.port_;
	//���̳߳���ѡ���߳�
	EventLoop* ioLoop = threadPool_->getNextLoop();
	//�������Ӷ���
	std::shared_ptr<ClientConnection> conn(new ClientConnection(ioLoop, connName, fd));
	ClientConnectionMap_[connName] = conn;

	//��ÿ���ͻ������ûص�����
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(closeCallback_);
	conn->setConnectionCallback(connectionCallback_);
	//һ��Ҫ�ŵ�io�̵߳��ã����̵߳�������
	//����ʵ���������  (�ͻ��˱��ָ��˹����̣߳�����ֻ���ڹ����̱߳�����)
	//���߹����̣߳����õ�ǰ�ͻ���
	ioLoop->runInLoop(std::bind(&ClientConnection::connectEstablished, conn));

}

//��ÿ���ͻ��˸��ĵĲ�������Ӧ�÷ŵ��ͻ��������߳�����ɵ���
//��ע�⣺����ǰ�ͻ��������߳�
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
	auto dt = nowTime - oldTime_;//������һ�ε�ʱ�侭�^
	oldTime_ = nowTime;
	//����ÿ���ͻ��˶������¼������Ӹ�Ч
	for(auto it = ClientConnectionMap_.begin(); it != ClientConnectionMap_.end();)
	{
		if(it->second->checkHeart(dt))//death
		{
			//�Ƴ�   connectDestory();   ClientConnection
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


//��Ҫ�رտͻ���
LightServer::~LightServer()
{
	loop_->assertInLoopThread();
	LOG << "TcpServer::~TcpServer";
	for (auto& item : ClientConnectionMap_)
	{
		ClientConnectionPtr conn(item.second);
		item.second.reset();
		//��ÿ��tcp�ͻ��˵��߳��Լ��ر�
		conn->getLoop()->runInLoop(
			std::bind(&ClientConnection::connectDestory, conn));
	}
	::close(listenFd);
}
