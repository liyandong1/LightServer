
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <vector>
#include <string>
#include <cstddef>

//为网络收发提供缓冲
class Buffer
{
private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];

	char* begin();
	const char* begin() const;
	void makeSpace(size_t len);

public:
	static const size_t kCheapPrepend;//方便在数据前面添加几个字节
	static const size_t kInitialSize;

	explicit Buffer(size_t initialSize = kInitialSize);
	void swap(Buffer& rhs);//便于缓冲交换
	
	//三个区间的大小
	size_t readableBytes() const;
	size_t writeableBytes() const;
	size_t prependableBytes() const;

	//定位到可读缓冲区
	const char* peek() const;
	const char* findCRLF() const;
	const char* findCRLF(const char* start) const;
	const char* findEOL() const;
	const char* findEOL(const char* start) const;

	//重定位读缓冲起始位置
	void retrieve(size_t len); 
	void retrieveUnitl(const char* end);
	void retrieveInt64();
	void retrieveInt32();
	void retrieveInt16();
	void retrieveInt8();
	void retrieveAll();

	//将可读缓冲区中的内容转化为字符串
	std::string retrieveAllAsString();
	std::string retrieveAsString(size_t len);
	//StringPiece toStringPiece() const

	//std::string toString();
	void append(const std::string& str);
	void append(const char* data, size_t len);
	void append(void* data, size_t len);

	//确保有足够的空间可写
	void ensureWriteableBytes(size_t len);

	char* beginWrite();
	const char* beginWrite() const;
	void hasWritten(size_t len);
	void unwrite(size_t len);

	//添加的是网络字节序，部分未实现
	void appendInt64(int64_t x);
	void appendInt32(int32_t x);
	void appendInt16(int16_t x);
	void appendInt8(int8_t x);

	//从缓冲区中读数据,实际交给peek去读
	int64_t readInt64();
	int32_t readInt32();
	int16_t readInt16();
	int8_t readInt8();

	//缓冲区中的数据都是网络序的
	//从缓冲区取出来提交给用户的数据应该转换为主机序
	int64_t peekInt64() const;
	int32_t peekInt32() const;
	int16_t peekInt16() const;
	int8_t peekInt8() const;

	//放在前面部分的数据也应该是网络序
	void prependInt64(int64_t x);
	void prependInt32(int32_t x);
	void prependInt16(int16_t x);
	void prependInt8(int8_t x);
	void prepend(const void* data, size_t len);

	//收缩
	void shrink(size_t reserve);

	//内部缓冲区大小
	size_t internalCapacity() const;

	//非阻塞网络编程提高效率，但是缓冲区不能太大
	//利用栈上的缓冲区，这也是双缓冲 readv writev
	size_t readFd(int fd, int* savedErrno);
	
	size_t writeFd(int fd);

	~Buffer();
};

#endif
