
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "Config.h"
#include <string>


class Process
{
public:
	int create_daemon(); //�����ػ�����
	void set_proc_name(char* argv, std::string proc_name);//���ý��̱���
};


#endif
