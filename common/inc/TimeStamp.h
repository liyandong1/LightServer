
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include <chrono>

using namespace std::chrono;

class Time
{
public:
	//��ȡ��ǰʱ�䣨���룩
	static time_t getNowInMillSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

class TimeStamp
{
private:
	//�߷ֱ���ʱ��
	time_point<high_resolution_clock> begin_;
public:
	TimeStamp();
	void update();
	//�Ѿ���ȥ������
	double getElapsedSecond();
	//�Ѿ���ȥ���ٺ��� 
	double getElapsedTimeInMilliSec();
	//�Ѿ���ȥ����΢��
	long long getElapsedTimeInMicrosec();

	~TimeStamp();
};

#endif
