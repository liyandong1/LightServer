
#include "TimeStamp.h"

TimeStamp::TimeStamp()
{
	update();
}

void TimeStamp::update()
{
	begin_ = high_resolution_clock::now();
}

//获取秒
double TimeStamp::getElapsedSecond()
{
	return this->getElapsedTimeInMicrosec() * 0.000001;
}

//获取毫秒 
double TimeStamp::getElapsedTimeInMilliSec()
{
	return this->getElapsedTimeInMicrosec() * 0.001;
}

//获取微秒
long long TimeStamp::getElapsedTimeInMicrosec()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - begin_).count();
}

TimeStamp::~TimeStamp() { }