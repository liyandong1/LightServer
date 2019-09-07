
#ifndef _MESSAGEBYTESTREAM_H
#define _MESSAGEBYTESTREAM_H

#include "MessageFormat.h"
#include "ByteStream.h"

class MessageRecvByteStream : public ByteStream
{
private:

public:
	//以一条具体的消息建立字节流
	MessageRecvByteStream(DataHeader* header):
	  ByteStream(reinterpret_cast<char*(header), header->dataLength + sizeof(*header))
	{
		push(header->dataLength);
		writeInt32();
	}
			
	
	~MessageRecvByteStream()
	{
		
	}
};


class MessageSendByteStream : public ByteStream
{
private:

public:
	//以一条具体的消息建立字节流
	MessageSendByteStream(DataHeader* header):
	  ByteStream(reinterpret_cast<char*(header), header->dataLength + sizeof(*header))
	{
		writeInt32(0); //预先占领消息长度所需要的空间
	}
			
	void sendCmd(uint32_t cmd)
	{
		write<uint16_t>(cmd);
	}
	
	~MessageSendByteStream()
	{
		
	}
};
#endif