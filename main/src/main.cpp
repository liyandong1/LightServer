
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <iostream>
#include "Process.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "TimeStamp.h"
#include "ClientConnection.h"
#include "LightServer.h"
#include "BufferParser.h"
#include "UserInfo.h"

int lanuchWithDeamon()
{
	//以守护进程方式启动
	if (Config::getInstance()->getNum("Daemon") == 1)
	{
		int cdaemonresult = Process().create_daemon();
		if (cdaemonresult == -1)//创建失败
		{
			LOG << errno << " " << "Process create daemon error ";
			return -1;
		}
		else if (cdaemonresult == 1)//父进程退出
		{
			return 0;
		}
	}
	return 1;
}

//用户名对应的信息
UserMap usersMap;



//实现三个回调函数
void onConnection(const ClientConnectionPtr& pClient)
{
	
}

void onClose(const ClientConnectionPtr& pClient)
{
	
}

//三个处理函数

void processRegister(const ClientConnectionPtr& pClient, DataHeader*& info)
{
	//为了方便找到tcp连接
	std::string name(reinterpret_cast<Register*>(info)->userName);
	UserInfo* usrInfo = new UserInfo(reinterpret_cast<Register*>(info)->passWord, pClient.get());
	usersMap.insert(name, usrInfo);
	
	//注册成功应该给客户端返回消息
	RegisterResult result;
	result.success = true;
	//不要跨线程调用
	pClient->getLoop()->runInLoop(\
	std::bind(&ClientConnection::sendMessage, pClient.get(), reinterpret_cast<char*>(&result), sizeof(result)));
}

void processLogin(const ClientConnectionPtr& pClient, DataHeader*& info)
{
	std::string name(reinterpret_cast<Register*>(info)->userName);
	LoginResult result;
	if(usersMap.find(name) != nullptr) //找到了
	{
		result.result = true;
	}
	else
	{
		result.result = false;
	}
	//不要跨线程调用
	pClient->getLoop()->runInLoop(\
	std::bind(&ClientConnection::sendMessage, pClient.get(), reinterpret_cast<char*>(&result), sizeof(result)));
}

void processLoginOut(const ClientConnectionPtr& pClient, DataHeader* info)
{
	std::string name(reinterpret_cast<LoginOut*>(info)->userName);
	usersMap.exit(name);
	pClient->connectDestory();
}

//发给在线的每一个客户端
void processChatMessage(DataHeader*& info)
{
	std::vector<ClientConnection*> clients = usersMap.getOnlineClients();
	for(size_t i = 0; i < clients.size(); i++)
	{
		clients[i]->getLoop()->runInLoop(\
		std::bind(&ClientConnection::sendMessage, clients[i], reinterpret_cast<char*>(info), sizeof(ChatMessage)));
	}
}


//每一个数据包都可以当作心跳包
void onMessage(const ClientConnectionPtr& pClient, Buffer* buffer, TimeStamp time)
{	
	
	std::cout << "onMessage() " << std::endl;
	//重置心跳时间
	//pClient->getLoop()->runInLoop(\
				std::bind(&ClientConnection::resetHeartTime, pClient.get()));
	
	BufferParser bufPaser(*buffer);
	DataHeader* result = nullptr;
	while(result == nullptr)
	{
		result = bufPaser.getParserResult();
	}
	//根据数据包类型执行对应的操作
	switch(result->cmd)
	{
		case CMD_REGISTER:
			processRegister(pClient, result);
			break;
		case CMD_LOGIN:
			processLogin(pClient, result);
			break;
		case CMD_LOGOUT:
			processLoginOut(pClient, result);
			break;
		case CMD_CHATMESSAGE:
			processChatMessage(result);
			break;
		case CMD_HEART:
			pClient->getLoop()->runInLoop(\
				std::bind(&ClientConnection::resetHeartTime, pClient.get()));
			break;
		default:
			break;
	}
	
	
	std::cout << reinterpret_cast<Login*>(result)->userName << std::endl;
	std::cout << reinterpret_cast<Login*>(result)->passWord << std::endl;
	
	std::cout << "Time used :" << time.getElapsedTimeInMicrosec() << endl;
	if(result != nullptr)
	{
		delete result;
		result = nullptr;
	}
}


int main(int argc, char* argv[])
{
	//装载配置文件
	Config::getInstance()->load("//home//book//code//LightServer//LightServer.conf");
	//设置日志路径
	//..........
	if (lanuchWithDeamon())
	{
		//设置进程标题
		Process().set_proc_name(argv[0], Config::getInstance()->getStr("Name"));
		
		EventLoop loop;
		LightServer server(&loop, 8000, 4);
		server.setConnectionCallback(onConnection);
		server.setMessageCallback(onMessage);
		server.setCloseCallback(onClose);
		server.listen();
		loop.loop();
	}
	return 0;
}
