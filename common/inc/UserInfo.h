#ifndef _USERINFO_H_
#define _USERINFO_H_

#include <string.h>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
//每个用户名对应的信息

class ClientConnection;

struct UserInfo
{
	bool isOnline;
	char passWord[32];
	ClientConnection* pClient;
	
	UserInfo(const char* pwd, ClientConnection* p)
	{
		isOnline = true;
		strcpy(passWord, pwd);
		pClient = p;
	}
};

class UserMap
{
private:
	std::mutex m_mutex;
	std::unordered_map<std::string, UserInfo*> users;
public:
	void insert(std::string name, UserInfo* ptr)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		if(users.find(name) == users.end())
		{
			users[name] = ptr;
		}
	}	
	
	UserInfo* find(std::string name)
	{
		UserInfo* ret = nullptr;
		for(auto it = users.begin(); it != users.end(); ++it)
		{
			if(it->first == name)
				ret = it->second;
		}
		return ret;
	}
	
	void exit(std::string name)
	{
		for(auto it = users.begin(); it != users.end(); ++it)
		{
			if(it->first == name)
				it->second->isOnline = false;
		}
	}
	
	//获取所有在线客户端对象
	std::vector<ClientConnection*> getOnlineClients()
	{
		vector<ClientConnection*> ret;
		for(auto it = users.begin(); it != users.end(); ++it)
		{
			if(it->second->isOnline == true)
				ret.push_back(it->second->pClient);
		}
		return ret;
	}
	
	size_t size()
	{
		return users.size();
	}
	
	~UserMap()
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		for(auto it = users.begin(); it != users.end(); ++it)
		{
			delete it->second;
		}
	}
};

#endif
