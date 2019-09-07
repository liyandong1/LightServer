
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "Config.h"
#include <string>


class Process
{
public:
	int create_daemon(); //创建守护进程
	void set_proc_name(char* argv, std::string proc_name);//设置进程标题
};


#endif
