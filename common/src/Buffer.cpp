
#include "Buffer.h"
#include "Endin.h"
#include "Logging.h"
#include <assert.h>
#include <memory.h>
#include <errno.h>
#include <sys/uio.h>
#include <algorithm>
#include <unistd.h>

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend = 8;//����������ǰ����Ӽ����ֽ�
const size_t Buffer::kInitialSize = 1024;

char* Buffer::begin()
{
	return &*buffer_.begin();
}

const char* Buffer::begin() const
{
	return &*buffer_.begin();
}

//��readerindex�ȽϿ����ʱ��Ų���ռ䵽ǰ��,�������ݲ���
void Buffer::makeSpace(size_t len)
{
	if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
	{
		buffer_.resize(writerIndex_ + len);
	}
	else
	{
		assert(kCheapPrepend < readerIndex_);
		size_t readable = readableBytes();
		std::copy(begin() + readerIndex_,
			      begin() + writerIndex_,
			      begin() + kCheapPrepend);
		//�����α�
		readerIndex_ = kCheapPrepend;
		writerIndex_ = readerIndex_ + readable;
		assert(readable == readableBytes());
	}
}

Buffer::Buffer(size_t initialSize):buffer_(kCheapPrepend + initialSize),
		readerIndex_(kCheapPrepend),writerIndex_(kCheapPrepend)
{
	assert(readableBytes() == 0);
	assert(writeableBytes() == initialSize);
	assert(prependableBytes() == kCheapPrepend);
}

void Buffer::swap(Buffer& rhs)
{
	buffer_.swap(rhs.buffer_);
	std::swap(readerIndex_, rhs.readerIndex_);
	std::swap(writerIndex_, rhs.writerIndex_);
}

size_t Buffer::readableBytes() const
{
	return writerIndex_ - readerIndex_;
}

size_t Buffer::writeableBytes() const
{
	return buffer_.size() - writerIndex_;
}
size_t Buffer::prependableBytes() const
{
	return readerIndex_;
}

//��λ���ɶ�������
const char* Buffer::peek() const
{
	return begin() + readerIndex_;
}

const char* Buffer::findCRLF() const
{
	const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
	return crlf = beginWrite() ? NULL : crlf;
}

//��ָ��λ�ú������
const char* Buffer::findCRLF(const char* start) const
{
	assert(peek() <= start);
	assert(start <= beginWrite());
	const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}

//memchr����ԭ��extern void *memchr(const void *buf, int ch, size_t count)��
//���ܣ���buf��ָ�ڴ������ǰcount���ֽڲ����ַ�ch��
const char* Buffer::findEOL() const
{
	const void* eol = memchr(peek(), '\n', readableBytes());
	return static_cast<const char*>(eol);
}

const char* Buffer::findEOL(const char* start) const
{
	assert(peek() <= start);
	assert(start <= beginWrite());
	const void* eol = memchr(start, '\n', beginWrite() - start);
	return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len)
{
	assert(len <= readableBytes());
	if (len < readableBytes())
	{
		readerIndex_ += len;
	}
	else
	{
		retrieveAll();
	}
}

void Buffer::retrieveUnitl(const char* end)
{
	assert(peek() <= end);
	assert(end <= beginWrite());
	retrieve(end - peek());
}

void Buffer::retrieveInt64()
{
	retrieve(sizeof(int64_t));
}

void Buffer::retrieveInt32()
{
	retrieve(sizeof(int32_t));
}

void Buffer::retrieveInt16()
{
	retrieve(sizeof(int16_t));
}

void Buffer::retrieveInt8()
{
	retrieve(sizeof(int8_t));
}

void Buffer::retrieveAll()
{
	readerIndex_ = kCheapPrepend;
	writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
	return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
	assert(len <= readableBytes());
	std::string result(peek(), len);
	retrieve(len);
	return result;
}

void Buffer::append(const std::string& str)
{
	append(&*str.begin(), str.size());
}

void Buffer::append(const char* data, size_t len)
{
	ensureWriteableBytes(len);
	std::copy(data, data + len, beginWrite());
	hasWritten(len);
}

void Buffer::append(void* data, size_t len)
{
	append(static_cast<const char*>(data), len);
}

void Buffer::ensureWriteableBytes(size_t len)
{
	if (writeableBytes() < len)
	{
		makeSpace(len);
	}
	assert(writeableBytes() >= len);
}

char* Buffer::beginWrite()
{
	return begin() + writerIndex_;
}

const char* Buffer::beginWrite() const
{
	return begin() + writerIndex_;
}

void Buffer::hasWritten(size_t len)
{
	assert(len <= writeableBytes());
	writerIndex_ += len;
}

void Buffer::unwrite(size_t len)
{
	assert(len <= readableBytes());
	writerIndex_ -= len;
}

void Buffer::appendInt64(int64_t x)
{
	//int64_t be64 = hostToNetwork64(x);
	append(&x, sizeof x);
}

void Buffer::appendInt32(int32_t x)
{
	//int32_t be32 = hostToNetwork32(x);
	append(&x, sizeof x);
}

void Buffer::appendInt16(int16_t x)
{
	//int16_t be16 = hostToNetwork16(x);
	append(&x, sizeof x);
}

void Buffer::appendInt8(int8_t x)
{
	append(&x, sizeof(x));
}

int64_t Buffer::readInt64()
{
	int64_t result = peekInt64();
	retrieveInt64();
	return result;
}

int32_t Buffer::readInt32()
{
	int32_t result = peekInt32();
	retrieveInt32();
	return result;
}

int16_t Buffer::readInt16()
{
	int16_t result = peekInt16();
	retrieveInt16();
	return result;
}

int8_t Buffer::readInt8()
{
	int8_t result = peekInt8();
	retrieveInt8();
	return result;
}

int64_t Buffer::peekInt64() const
{
	assert(readableBytes() >= sizeof(int64_t));
	int64_t be64 = 0;
	::memcpy(&be64, peek(), sizeof be64);
	//return networkToHost64(be64);
	return be64;
}

int32_t Buffer::peekInt32() const
{
	assert(readableBytes() >= sizeof(int32_t));
	int32_t be32 = 0;
	::memcpy(&be32, peek(), sizeof be32);
	//return networkToHost32(be32);
	return be32;
}

int16_t Buffer::peekInt16() const
{
	assert(readableBytes() >= sizeof(int16_t));
	int16_t be16 = 0;
	::memcpy(&be16, peek(), sizeof be16);
	//return networkToHost16(be16);
	return be16;
}

int8_t Buffer::peekInt8() const
{
	assert(readableBytes() >= sizeof(int8_t));
	int8_t x = *peek();
	return x;
}

void Buffer::prependInt64(int64_t x)
{
	//int64_t be64 = hostToNetwork64(x);
	prepend(&x, sizeof x);
}

void Buffer::prependInt32(int32_t x)
{
	//int32_t be32 = hostToNetwork32(x);
	prepend(&x, sizeof x);
}

void Buffer::prependInt16(int16_t x)
{
	//int16_t be16 = hostToNetwork16(x);
	prepend(&x, sizeof x);
}

void Buffer::prependInt8(int8_t x)
{
	prepend(&x, sizeof x);
}

void Buffer::prepend(const void* data, size_t len)
{
	assert(len <= prependableBytes());
	readerIndex_ -= len;
	const char* d = static_cast<const char*>(data);
	std::copy(d, d + len, begin() + readerIndex_);
}

void Buffer::shrink(size_t reserve)
{
	buffer_.shrink_to_fit();
}

size_t Buffer::internalCapacity() const
{
	return buffer_.capacity();
}

//readv  writev��������ջ�ϻ����ȡ����
size_t Buffer::readFd(int fd, int* savedErrno)
{
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writeable = writeableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writeable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof(extrabuf);

	//����������д�ռ��㹻�������õ�ջ�ϵĿռ�
	//��Ԥ�����屻���ã���������ȡ128k
	const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;//�Ƿ����ö��⻺��
	const size_t n = readv(fd, vec, iovcnt);

	if (n < 0)
	{
		*savedErrno = errno;
	}
	else if (static_cast<size_t>(n) < writeable)
	{
		writerIndex_ += n;
	}
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writeable);//�����ݷŵ���ǰ����,�Զ�����
	}

	return n;
}

size_t Buffer::writeFd(int fd)
{
	//�ѿɶ�������ȫ��д��ȥ
	int n = write(fd, peek(), readableBytes());
	if(n < 0)
	{
		LOG_ERROR << "Buffer::writeFd error";
	}
	else
	{
		retrieve(n);
	}
	return n;
}

Buffer::~Buffer()
{

}
