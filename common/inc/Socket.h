#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <string>

struct InetAddress
{
	std::string ip_;
	int port_;
};

class Socket
{
private:
	const int fd_;
	int port_;
public:
	explicit Socket(int port);
	explicit Socket(std::string Fd);
	int fd() const;
	void bindAddress(const std::string& addr = "");
	void listen();
	int accept(InetAddress* peeraddr);//���ضԶ˵�ַ��Ϣ
	void shutdownWrite();
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);
	int getSocketError();
	void close();
	~Socket();
};

#endif
