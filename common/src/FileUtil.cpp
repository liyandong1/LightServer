

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileUtil.h"

AppendFile::AppendFile(std::string filename)
	:fp_(fopen(filename.c_str(), "ae"))
{
	//���ļ�֮��ʹ���û��ṩ�Ļ�����
	setbuffer(fp_, buffer_, sizeof(buffer_));
}

//д���ļ�
size_t AppendFile::write(const char* logline, size_t len)
{
	//��������汾��fwrite
	return fwrite_unlocked(logline, 1, len, fp_);
}

void AppendFile::flush()
{
	fflush(fp_);
}


void AppendFile::append(const char* logline, const size_t len)
{
	size_t n = this->write(logline, len);
	size_t remain = len - n;

	//���һ��δд�꣬��֤���ݳ���д��
	while (remain > 0)
	{
		size_t x = this->write(logline + n, remain);
		if (x == 0)//д����ɻ���ʧ��
		{
			int err = ferror(fp_);
			if (err)
				fprintf(stderr, "AppendFile::append() failed !\n");
			break;
		}
		n += x;
		remain = len - n;
	}
}

AppendFile::~AppendFile()
{
	fclose(fp_);
}