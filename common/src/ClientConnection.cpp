
#include <unistd.h>
#include "Logging.h"
#include "Socket.h"
#include "Channel.h"
#include "TimeStamp.h"
#include "EventLoop.h"
#include "ClientConnection.h"

#include <iostream>
using namespace std;

ClientConnection::ClientConnection(EventLoop* loop, const std::string& name, int connfd) :
	loop_(loop), clientChannel_(new Channel(loop, connfd)),
	clientSocket_(new Socket(std::to_string(connfd))), name_(name)
{
	clientChannel_->setReadCallback(std::bind(&ClientConnection::handleRead, this));
	clientChannel_->setWriteCallback(std::bind(&ClientConnection::handleWrite, this));
	clientChannel_->setErrorCallback(std::bind(&ClientConnection::handleClose, this));
	clientChannel_->setCloseCallback(std::bind(&ClientConnection::handleError, this));

	clientSocket_->setKeepAlive(true);
	resetHeartTime();
}

void ClientConnection::setMessageCallback(const MessageCallback& cb)
{
	messageCallback_ = cb;
}

void ClientConnection::setCloseCallback(const CloseCallback& cb)
{
	closeCallback_ = cb;
}

void ClientConnection::setConnectionCallback(const ConnectionCallback& cb)
{
	connectionCallback_ = cb;
}

void ClientConnection::handleRead()
{
	int savedErrno;
	TimeStamp receiveTime;
	size_t n = inputBuffer_.readFd(clientSocket_->fd(), &savedErrno);
	if (n > 0)
	{
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		handleError();
	}
}

//��д��������ݷ��ͳ�ȥ��ע��epoll״̬
void ClientConnection::handleWrite()
{
	int fd = clientSocket_->fd();
	size_t dataSize = outputBuffer_.readableBytes();
	size_t n = outputBuffer_.writeFd(fd);
	LOG << "write bytes() " << n;
	
	if(n == dataSize)//д���ˣ����Բ��ü���д�¼�
	{
		clientChannel_->disableWriting();
		clientChannel_->enableReading();
	}
	else//??�Ƿ���Ҫ�ٴ�ע��д�¼�
	{
		clientChannel_->enableWriting();
	}
}

void ClientConnection::handleClose()
{
	loop_->assertInLoopThread();
	LOG << "ClientConnection::handleClose name = " << name_;
	//���ر��ļ����������ȵ�����
	closeCallback_(shared_from_this()); //closeCallback_����LightServe
	connectDestory();
	LOG << name_ << "connectDestory()";
}

void ClientConnection::handleError()
{
	int err = clientSocket_->getSocketError();
	LOG << "ClientConnection::handleError [" << name_ << "] - SO_ERROR =" << err;
	clientChannel_->disableAll();
}

EventLoop* ClientConnection::getLoop()
{
	return loop_;
}

void ClientConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	clientChannel_->tie(shared_from_this()); //ÿ��Channel������뱨ȷ����Client������ϵ��һ��
	clientChannel_->enableReading();
	
	//�½��ͻ��˶���,֪ͨ������
	connectionCallback_(shared_from_this());
}

void ClientConnection::connectDestory()
{
	loop_->assertInLoopThread();
	clientChannel_->disableAll();
	clientChannel_->remove();
}

void ClientConnection::sendMessage(const char* msg, size_t len)
{
	outputBuffer_.append(msg, len);
	//ע��ɶ��¼�
	clientChannel_->enableWriting();
}

void ClientConnection::resetHeartTime() 
{
	heartTime_ = 0;
}

bool ClientConnection::checkHeart(time_t dt)
{
	heartTime_ += dt;
	if(heartTime_ >= HEART_TIME)
	{
		//����
		LOG << "checkHeart faild, this client die...";
		return true;
	}
	
	return false;
}

ClientConnection::~ClientConnection()
{
	LOG << "Client[" << name_ << "] dtor";
}
