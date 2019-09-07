
#ifndef _BUFFERPARSER_H_
#define _BUFFERPARSER_H_

#include "Buffer.h"
#include "MessageFormat.h"
#include "Logging.h"

#include <iostream>
using namespace std;

class BufferParser
{
private:
	Buffer& buffer;
	DataHeader* currentHeader;  //当前正在解析的包头结构
	DataHeader* parserResult;   //当前解析的结果
	//解析注册信息
	void parserRegisterData()
	{
		if(buffer.readableBytes() < currentHeader->dataLength)  //currentHeader->dataLength
		{
			parserResult = nullptr;
			return;   //不够字节数，等待下一次解析
		}
		parserResult = new Register;
		parserResult->dataLength = currentHeader->dataLength;
		parserResult->cmd = currentHeader->cmd;
		//读出账户密码
		strcpy(reinterpret_cast<Register*>(parserResult)->userName, buffer.retrieveAsString(32).c_str());
		strcpy(reinterpret_cast<Register*>(parserResult)->passWord, buffer.retrieveAsString(32).c_str());
	}
	
	//解析登录信息
	void parserLoginData()
	{
	
		if(buffer.readableBytes() < currentHeader->dataLength)  //currentHeader->dataLength
		{
			parserResult = nullptr;
			return;   //不够字节数，等待下一次解析
		}
		parserResult = new Login;
		parserResult->dataLength = currentHeader->dataLength;
		parserResult->cmd = currentHeader->cmd;
		//读出账户密码
		strcpy(reinterpret_cast<Login*>(parserResult)->userName, buffer.retrieveAsString(32).c_str());
		strcpy(reinterpret_cast<Login*>(parserResult)->passWord, buffer.retrieveAsString(32).c_str());
	}
	
	//解析登出信息
	void parserLoginOutData()
	{
		if(buffer.readableBytes() < currentHeader->dataLength)  //currentHeader->dataLength
		{
			parserResult = nullptr;
			return;   //不够字节数，等待下一次解析
		}
		parserResult = new LoginOut;
		parserResult->dataLength = currentHeader->dataLength;
		parserResult->cmd = currentHeader->cmd;
		//读出用户名
		strcpy(reinterpret_cast<LoginOut*>(parserResult)->userName, buffer.retrieveAsString(32).c_str());
	}
	
	//解析聊天信息
	void parserChatMessage()
	{
		if(buffer.readableBytes() < currentHeader->dataLength)  //currentHeader->dataLength
		{
			parserResult = nullptr;
			return;   //不够字节数，等待下一次解析
		}
		parserResult = new ChatMessage;
		//读出信息
		
		strcpy(reinterpret_cast<ChatMessage*>(parserResult)->userName, buffer.retrieveAsString(32).c_str());
		strcpy(reinterpret_cast<ChatMessage*>(parserResult)->message, buffer.retrieveAsString(128).c_str());
	}
	
	void parserHeartMessage()
	{
		parserResult = new Heart;
		parserResult->dataLength = currentHeader->dataLength;
		parserResult->cmd = currentHeader->cmd;
	}
public:
	BufferParser(Buffer& buff):buffer(buff)
	{
		currentHeader = nullptr;
		parserResult = nullptr;
	}
	
	//得到每次解析的结果,解析不成功，返回nullptr
	DataHeader* getParserResult()
	{
		//先删掉上一次解析的结果
		if(currentHeader == nullptr)   //可能上一次的解析还没有完成
		{
			//先收一个包头的结构
			if(buffer.readableBytes() < sizeof(DataHeader))
				return nullptr; //不满足包头的长度
			currentHeader = new DataHeader;//delete
			currentHeader->dataLength = buffer.readInt32();
			currentHeader->cmd =  static_cast<CMD>(buffer.readInt32());
		}
		
		//delete掉上一次解析的结果
		if(parserResult != nullptr)
		{
			delete parserResult;
			parserResult = nullptr;
		}
		
		//根据类型判断如何解析
		
		switch(currentHeader->cmd)
		{
			case CMD_REGISTER:
				parserRegisterData();
				break;
			case CMD_LOGIN:
				parserLoginData();
				break;
			case CMD_LOGOUT:
				parserLoginOutData();
				break;
			case CMD_CHATMESSAGE:
				parserChatMessage();
				break;	
			case CMD_HEART:
				parserHeartMessage();
			default:
				break;
		};
		
		//说明当前数据包被解析完了
		if(parserResult != nullptr)
		{
			delete currentHeader;
			currentHeader = nullptr;
		}
		return parserResult;
	}
	
	~BufferParser()
	{
		if(currentHeader != nullptr)
			delete currentHeader;
	}
};


#endif
