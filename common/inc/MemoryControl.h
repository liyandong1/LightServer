
#ifndef _MEMORYCONTROL_H_
#define _MEMORYCONTROL_H_

#include "MemoryManger.h"
//ͨ���ڴ����Ԫ�����ڴ�

void* operator new(size_t size);
void operator delete(void* p) noexcept;

void* operator new[](size_t size);
void operator delete[](void* p) noexcept;

#endif
