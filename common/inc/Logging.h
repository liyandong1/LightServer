
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string.h>
#include <string>
#include <stdio.h>
#include "LogStream.h"

enum class Level { Debug = 0, Info, Warning, Error, Fatal };//��־����

class Logger
{
private:
	class Impl
	{
	public:
		std::string basename_;
		int line_;
		Level level_;//ת��Ϊ�ַ���
		LogStream stream_;

		Impl(const char* fileName, int line, Level level);
		//���ڵ�ǰ��������ͷ�����ʱ����Ϣ,������Ϣ
		void formatTime();
	};

	Impl impl_;//���Զ����ṩһ���ʽ����ɺ�Ļ���
	static std::string logFileName_;
public:
	Logger(const char* fileName, int line, Level level);
	LogStream& stream();
	~Logger();

	static void setLogFileName(std::string fileName);
	static std::string getLogFileName();
};

#define LOG_DEBUG   Logger(__FILE__, __LINE__, Level::Debug).stream()
#define LOG         Logger(__FILE__, __LINE__, Level::Info).stream()
#define LOG_WARNING Logger(__FILE__, __LINE__, Level::Warning).stream()
#define LOG_ERROR   Logger(__FILE__, __LINE__, Level::Error).stream()
#define LOG_FATAL   Logger(__FILE__, __LINE__, Level::Fatal).stream()

#endif
