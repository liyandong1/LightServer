
#ifndef _FILEUTIL_H_
#define _FILEUTIL_H_

#include <string>
#include "noncopyable.h"

const int MAX_BUFFER_SIZE = 64 * 1024;

//Ϊ�ļ���װһ��
class AppendFile : public noncopyable
{
private:
	FILE* fp_;
	char buffer_[MAX_BUFFER_SIZE];//Ϊд�ļ��ṩ��Ӧ�ò㻺����
	//��ʵ���ļ�д
	size_t write(const char* logline, size_t len);
public:
	explicit AppendFile(std::string filename);
	void flush();
	//append���ĺ������ṩ�������߽ӿڣ���֤��ʵ�İ�����ȫ��д�뵽����
	void append(const char* logline, const size_t len);
	~AppendFile();
};

#endif