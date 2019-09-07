
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include <chrono>

using namespace std::chrono;

class Time
{
public:
	//获取当前时间（毫秒）
	static time_t getNowInMillSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

class TimeStamp
{
private:
	//高分辨率时钟
	time_point<high_resolution_clock> begin_;
public:
	TimeStamp();
	void update();
	//已经过去多少秒
	double getElapsedSecond();
	//已经过去多少毫秒 
	double getElapsedTimeInMilliSec();
	//已经过去多少微秒
	long long getElapsedTimeInMicrosec();

	~TimeStamp();
};

#endif
