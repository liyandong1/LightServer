
/**
	����һ��log�̣߳�����ר�Ž�logд��logfile
	Ӧ��˫���弼����ʵ�������Ŀ黺��
*/

#ifndef _ASYNCLOGGING_H_
#define _ASYNCLOGGING_H_

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

#include "noncopyable.h"
#include "LogStream.h"
#include "CountDownLatch.h"


//�첽io����
class AsyncLogging : public noncopyable
{
private:
	using Buffer = FixedBuffer<kLargeBuffer>; //��װ��һ�黺������
	using BufferPtr = std::shared_ptr<Buffer>;
	using BufferPtrVector = std::vector<BufferPtr>;

	std::string basename_; //�ļ���
	const int flushInterval_;
	bool running_;

	std::thread thread_; //io�߳�
	std::mutex mutex_;
	std::condition_variable cond_;

	BufferPtr currentBuffer_;//��ǰ����ʹ�õĻ���
	BufferPtr nextBuffer_; //��һ��Ԥ������
	BufferPtrVector buffers_;
	CountDownLatch latch_;


	void threadFunc();// io�߳�ִ�к���,����̹߳���
public:
	AsyncLogging(const std::string basename, int flushInterval = 2);

	//ֻ�ǰ����ݷŵ���ǰ����������,��ǰ���̵߳���
	void append(const char* logline, int len);
	void start(); //���߳̿�ʼio
	void stop();  //���߳�ֹͣio
	~AsyncLogging();
};

#endif