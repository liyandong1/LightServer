
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Process.h"
#include "Logging.h"

//�����ػ�����
int Process::create_daemon()
{
	//(1)����umask(0)��Ҫ�����ػ����̵��ļ�Ȩ��
	umask(0);
	//(2)�����ӽ��̣��������˳�
	switch (fork())
	{
	case -1:   //�����ӽ���ʧ��
		LOG << errno << " " << "create_daemon() fork error";
		return -1;
	case 0:    //�ӽ���
		break;
	default:
		return 1; //�����̷��أ������̸��ݷ���ֵ�ͷ���Դ
	}

	//(3)�����»Ự�������ն�
	if (setsid() == -1)
	{
		LOG << errno << " " << "create_daemon() setsid error";
		return -1;
	}

	//(4)�ѵ�ǰ����Ŀ¼����Ϊ��Ŀ¼
	chdir("/");

	//(5)��null�豸��Ȼ���ض�����������ļ�������
	int fd = open("/dev/null", O_RDWR);
	if (fd == -1)
	{
		LOG << errno << " " << "create_daemon() open error";
		return -1;
	}
	//�ض����׼���룬��׼����ļ�������
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		LOG << errno << " " << "create_daemon() dup2 error";
		return -1;
	}

	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		LOG << errno << " " << "create_daemon() dup2 error";
		return -1;
	}
	if (fd > STDERR_FILENO)
	{
		if (close(fd) == -1)
		{
			LOG << errno << " " << "create_daemon() close error";
			return -1;
		}
	}
	return 0;  //�ӽ��̷���
}

void Process::set_proc_name(char* argv, std::string proc_name)
{
	memset(argv, 0, strlen(argv) + 1);
	strcpy(argv, proc_name.c_str());
}