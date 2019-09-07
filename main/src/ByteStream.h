
#ifndef _BYTESTREAM_H_
#define _BYTESTREAM_H_

//真正的读写操作都是封装memcpy
#include <cstdint>

class ByteStream
{
protected:
	char* pBuf_;
	int size_;
	int writePos_;//已经写入数据的尾部位置，已有数据的长度
	int readPos_;//已经读取数据的尾部位置
	
	bool bDel_;  //内存块是否应该被释放,默认为true
public:
	inline bool canRead(int n)
	{
		return size_ - readPos_ >= n;
	}
	
	inline bool canWrite()
	{
		return size_ - writePos_ >= n;
	}
	
	inline void push(int n)
	{
		writePos_ += n;
	}
	
	inline void pop()
	{
		readPos_ -= n;
	}
	
	//使用已有数据块
	ByteStream(char* pData, int size, bool bDelete = false):size_(size)
	{
		bDel_ = bDelete;
		pBuf_ = pData;
	}
	
	ByteStream(int size = 1024):size_(size)
	{
		bDel_ = true;
		pBuf_ = new char[size_];
	}
	
	virtual ~ByteStream()
	{
		if(pBuf_ != nullptr && bDel_ != nullptr)
		{
			delete[] pBuf_;
			pBuf_ = nullptr;
		}
	}
	
	char* data()
	{
		return pBuf_;
	}
	
	
	template <typename T>
	bool read(T& n, bool offset = true)
	{
		//计算要读取数据的长度
		int len = sizeof(T);
		//判断能不能读
		if(readPos_ + len <= size_)
		{
			memcpy(&n, pBuf_ + readPos_, len);
			if(offset == true)
				readPos_ += len;
			return true;
		}
		return false;
	}
	
	int8_t readInt8()
	{
		int8_t n = 0;
		read(n);
		return n;
	}
	
	int16_t readInt16()
	{
		int16_t n = 0;
		read(n);
		return n;
	}
	
	int32_t readInt32()
	{
		int32_t n = 0;
		read(n);
		return n;
	}
	
	float readFloat()
	{
		float n = 0;
		read(n);
		return n;
	}
	
	double readDouble()
	{
		double n = 0;
		read(n);
		return n;
	}
	
	//读取数组
	template <typename T>
	uint32_t readArray(T& pArray, uint32_t len)
	{
		uint32_t slen = 0; //比较长度是否相同
		read(slen, false);//注意标记位置已经偏移
		if(slen <= len)
		{
			uint32_t size = sizeof(T) * slen + sizeof(uint32_t);
			if(readPos_ + size <= size_)
			{
				memcpy(pArray, pBuf_ + readPos_, size);
				readPos_ += size;
				return slen;
			}
		}
		return 0;
	}
	
	//写基础数据
	template <typename T>
	bool write(T n)
	{
		//判断能不能写入
		//写入什么位置
		if(writePos_ + sizeof(T) <= size_)
		{
			memcpy(pBuf_ + writePos_, &n, sizeof(T));
			pBuf_ += sizeof(T);
			return;
		}
		return false;
	}
	
	//写数组
	template <typename T>
	bool writeArray(T* pData, uint32_t len)
	{
		//计算要写入的数据大小,包括数组的长度
		int Len = sizeof(T) * len + sizeof(int32_t);
		//判断能不能写入
		if(writePos_ + Len <= size_)
		{
			//写数组的时候也要把数组的长度写入进去
			writeInt32(len);
			memcpy(pBuf_ + writePos_, pData, Len);
			writePos_ += Len;
			return true;
		}
		return false;
	}
	
	bool writeInt8(int8_t n)
	{
		return write(n);
	}
	
	bool writeInt16(int16_t n)
	{
		return write(n);
	}
	
	bool writeInt32(int32_t n)
	{
		return write(n);
	}
	
	bool writeFloat(float n)
	{
		return write(n);
	}
	
	bool writeDouble()
	{
		return write(n);
	}
};

#endif