
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Process.h"
#include "Logging.h"

//创建守护进程
int Process::create_daemon()
{
	//(1)调用umask(0)不要限制守护进程的文件权限
	umask(0);
	//(2)创建子进程，父进程退出
	switch (fork())
	{
	case -1:   //创建子进程失败
		LOG << errno << " " << "create_daemon() fork error";
		return -1;
	case 0:    //子进程
		break;
	default:
		return 1; //父进程返回，主流程根据返回值释放资源
	}

	//(3)创建新会话，脱离终端
	if (setsid() == -1)
	{
		LOG << errno << " " << "create_daemon() setsid error";
		return -1;
	}

	//(4)把当前工作目录设置为根目录
	chdir("/");

	//(5)打开null设备，然后重定向输入输出文件描述符
	int fd = open("/dev/null", O_RDWR);
	if (fd == -1)
	{
		LOG << errno << " " << "create_daemon() open error";
		return -1;
	}
	//重定向标准输入，标准输出文件描述符
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
	return 0;  //子进程返回
}

void Process::set_proc_name(char* argv, std::string proc_name)
{
	memset(argv, 0, strlen(argv) + 1);
	strcpy(argv, proc_name.c_str());
}