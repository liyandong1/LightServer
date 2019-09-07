
#include <thread>
#include <assert.h>
#include <time.h>
#include <mutex>
#include <chrono>
#include <string>
#include <sstream>
#include <map>
#include "Logging.h"
#include "AsyncLogging.h"


#include <iostream>   
using namespace std;

static std::map<Level, const char*> LogLevelMap
{
	{Level::Debug, "Debug"},
	{Level::Info, "Info"},
	{Level::Warning, "Warning"},
	{Level::Error, "Error"},
	{Level::Fatal, "Fatal"}
};

//�ڶ��̳߳���ִ��ʱ��ĳЩ����ֻ��Ҫ��һ��
//call_once�ܱ�֤����ֽ������һ��

static std::once_flag onceFlag;//ϵͳ��һ��״̬λ������ȷ�������Ƿ񱻵��ù�
static AsyncLogging* AsyncLogger_;//��ֻ�ܱ�����һ��
std::string Logger::logFileName_ = "Server.log";

//��ʼ��AsyncLogging����ȷ��ֻ����һ�����
//�������ֻ�ܱ�����һ��

void once_init()
{
	AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
	AsyncLogger_->start();
}

//ת������AsyncLogger_�첽��־��������д����
void output(const char* msg, int len)
{
	std::call_once(onceFlag, once_init);
	//������Async���д
	AsyncLogger_->append(msg, len);
}


Logger::Impl::Impl(const char* fileName, int line, Level level)
	:basename_(fileName),line_(line),level_(level),stream_()
{
	formatTime();
}

//����C++11
void Logger::Impl::formatTime()
{
	//���ڵ�ǰ��������ͷ�����ʱ����Ϣ
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm* p_time = localtime(&currentTime);
	char str_t[26] = { 0 };
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S   ", p_time);
	stream_ << str_t;
	//����ӵȼ���Ϣ
	stream_ << "[Level::" << LogLevelMap[level_] << "]   ";
}

Logger::Logger(const char* fileName, int line, Level level)
	:impl_(fileName, line, level)
{

}

LogStream& Logger::stream()
{
	return impl_.stream_;
}

Logger::~Logger()
{
	//�õ��߳�id
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	
	impl_.stream_ << "   -- " << impl_.basename_ << ':' << impl_.line_ << 
					" ThreadId:" << oss.str() << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	output(buf.data(), buf.length());
}

void Logger::setLogFileName(std::string fileName)
{
	logFileName_ = fileName;
}

std::string Logger::getLogFileName()
{
	return logFileName_;
}
