
#include "TimeStamp.h"

TimeStamp::TimeStamp()
{
	update();
}

void TimeStamp::update()
{
	begin_ = high_resolution_clock::now();
}

//��ȡ��
double TimeStamp::getElapsedSecond()
{
	return this->getElapsedTimeInMicrosec() * 0.000001;
}

//��ȡ���� 
double TimeStamp::getElapsedTimeInMilliSec()
{
	return this->getElapsedTimeInMicrosec() * 0.001;
}

//��ȡ΢��
long long TimeStamp::getElapsedTimeInMicrosec()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - begin_).count();
}

TimeStamp::~TimeStamp() { }