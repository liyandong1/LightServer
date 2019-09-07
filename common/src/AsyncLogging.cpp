
#include <chrono>
#include "LogFile.h"
#include "AsyncLogging.h"

#include <iostream>
using namespace std;

AsyncLogging::AsyncLogging(const std::string basename, int flushInterval)
	:basename_(basename), flushInterval_(flushInterval), running_(false),
	thread_(std::bind(&AsyncLogging::threadFunc, this)), mutex_(),
	currentBuffer_(new Buffer),nextBuffer_(new Buffer),buffers_(),latch_(1)
{
	assert(basename_.size() > 1);
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);//Ԥ���ռ�
}

void AsyncLogging::threadFunc()
{
	{
		//�ȴ�start����ִ�У��̺߳�����ִ��
		std::unique_lock<std::mutex> lk(mutex_);
		cond_.wait(lk, [this]{return running_;});  //�ȴ�satrt�˲�����ִ��
		latch_.countDown();//�̺߳������������ˣ����start���Է���
	}
	
	//���������־����ļ�
	LogFile output(basename_);//ͨ����д��־
	
	//��������黺�壬ʵ�������Ļ���
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer1->bzero();
	
	//��Ҫд����̵����л��嶼���Ǹ�����
	BufferPtrVector buffersToWrite;
	buffersToWrite.reserve(16);//Ԥ���ռ�
	
	//�߳�ѭ����ִ��д�������
	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		//��������Ҫ�ٽ�
		{
			std::unique_lock<std::mutex> lk(mutex_);
			if (buffers_.empty())//û����Ҫд��
			{
				//����ÿ��flushInterval_ sдһ�δ��̣��վ�˯��
				cond_.wait_for(lk, std::chrono::seconds(flushInterval_));
			}
			buffers_.push_back(currentBuffer_);
			currentBuffer_.reset();

			currentBuffer_ = std::move(newBuffer1);

			//Ϊ�˺���Ĵ����ܹ���ȫ�ķ���buffersToWrite
			buffersToWrite.swap(buffers_);

			//����ǰ���ٽ����ĳ��ȣ�����ǰ�˷��仺��Ĵ���
			if (!nextBuffer_)
			{
				nextBuffer_ = std::move(newBuffer2);
			}

		}

			assert(!buffersToWrite.empty());
			
			//
			if (buffersToWrite.size() > 25)
				buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
			
			//д�뵽�ļ�����ʵ�ǻ��壩
			for (size_t i = 0; i < buffersToWrite.size(); ++i)
			{
				output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
			}

			if (buffersToWrite.size() > 2)
				buffersToWrite.resize(2);

			//��buffersToWrite����Ļ����滻��newBuffer
			//���Ȼ������
			if (!newBuffer1)
			{
				assert(!buffersToWrite.empty());
				newBuffer1 = buffersToWrite.back();
				buffersToWrite.pop_back();
				newBuffer1->reset();
			}

			if (!newBuffer2)
			{
				assert(!buffersToWrite.empty());
				newBuffer2 = buffersToWrite.back();
				buffersToWrite.pop_back();
				newBuffer2->reset();
			}

			buffersToWrite.clear();
			output.flush();//д������
		}
	output.flush();
}

void AsyncLogging::append(const char* logline, int len)
{
	std::unique_lock<std::mutex> lk(mutex_);
	if (currentBuffer_->avail() > len)
	{//��ǰ�����ܷ���
		currentBuffer_->append(logline, len);
	}
	else//��һ�黺��
	{
		buffers_.push_back(currentBuffer_);//�������
		currentBuffer_.reset();

		//һ��Ҫ�ҵ���һ����û���
		if (nextBuffer_)
			currentBuffer_ = std::move(nextBuffer_);
		else
			currentBuffer_.reset(new Buffer);
		currentBuffer_->append(logline, len);
		cond_.notify_all();   //֪ͨio�߳̿��Խ���д���̵�io������
	}
	
}

//Ҫע��thread����������������
void AsyncLogging::start()
{
	running_ = true;
	cond_.notify_all();// �������̻߳���
	latch_.wait();//Ϊ��ȷ��threadFunc������ִ���ˣ���ǰsatrt�����ŷ���
}

void AsyncLogging::stop()
{
	running_ = false;
	cond_.notify_all(); //�������е��߳�
	thread_.join(); //�����߳�
}

AsyncLogging::~AsyncLogging()
{
	if (running_)
		stop();
}
