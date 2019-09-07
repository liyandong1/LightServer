
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <vector>
#include <string>
#include <cstddef>

//Ϊ�����շ��ṩ����
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
	static const size_t kCheapPrepend;//����������ǰ����Ӽ����ֽ�
	static const size_t kInitialSize;

	explicit Buffer(size_t initialSize = kInitialSize);
	void swap(Buffer& rhs);//���ڻ��彻��
	
	//��������Ĵ�С
	size_t readableBytes() const;
	size_t writeableBytes() const;
	size_t prependableBytes() const;

	//��λ���ɶ�������
	const char* peek() const;
	const char* findCRLF() const;
	const char* findCRLF(const char* start) const;
	const char* findEOL() const;
	const char* findEOL(const char* start) const;

	//�ض�λ��������ʼλ��
	void retrieve(size_t len); 
	void retrieveUnitl(const char* end);
	void retrieveInt64();
	void retrieveInt32();
	void retrieveInt16();
	void retrieveInt8();
	void retrieveAll();

	//���ɶ��������е�����ת��Ϊ�ַ���
	std::string retrieveAllAsString();
	std::string retrieveAsString(size_t len);
	//StringPiece toStringPiece() const

	//std::string toString();
	void append(const std::string& str);
	void append(const char* data, size_t len);
	void append(void* data, size_t len);

	//ȷ�����㹻�Ŀռ��д
	void ensureWriteableBytes(size_t len);

	char* beginWrite();
	const char* beginWrite() const;
	void hasWritten(size_t len);
	void unwrite(size_t len);

	//��ӵ��������ֽ��򣬲���δʵ��
	void appendInt64(int64_t x);
	void appendInt32(int32_t x);
	void appendInt16(int16_t x);
	void appendInt8(int8_t x);

	//�ӻ������ж�����,ʵ�ʽ���peekȥ��
	int64_t readInt64();
	int32_t readInt32();
	int16_t readInt16();
	int8_t readInt8();

	//�������е����ݶ����������
	//�ӻ�����ȡ�����ύ���û�������Ӧ��ת��Ϊ������
	int64_t peekInt64() const;
	int32_t peekInt32() const;
	int16_t peekInt16() const;
	int8_t peekInt8() const;

	//����ǰ�沿�ֵ�����ҲӦ����������
	void prependInt64(int64_t x);
	void prependInt32(int32_t x);
	void prependInt16(int16_t x);
	void prependInt8(int8_t x);
	void prepend(const void* data, size_t len);

	//����
	void shrink(size_t reserve);

	//�ڲ���������С
	size_t internalCapacity() const;

	//���������������Ч�ʣ����ǻ���������̫��
	//����ջ�ϵĻ���������Ҳ��˫���� readv writev
	size_t readFd(int fd, int* savedErrno);
	
	size_t writeFd(int fd);

	~Buffer();
};

#endif
