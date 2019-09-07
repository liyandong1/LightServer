
/**
��һ����װAppendFile�࣬����һ��ѭ������
ÿ����ô��ξ���flushһ��
*/

#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include <mutex>
#include <memory>
#include <string>
#include "FileUtil.h"
#include "noncopyable.h"


class LogFile : public noncopyable
{
private:
	int count_;
	std::unique_ptr<std::mutex> mutex_;
	std::unique_ptr<AppendFile> file_;//��ʵ���ļ�

	const int flushEveryN_;
	const std::string basename_;

	//��������
	void append_unlocked(const char* logline, int len);
public:
	LogFile(const std::string& basename, int flushEveryN = 1024);
	void append(const char* logline, int len);
	void flush();
	//bool rollFile();
	~LogFile();
};


#endif