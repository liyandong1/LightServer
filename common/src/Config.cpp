//@Athor LiYanDong
//@Email 416620418@qq.com
#include "Process.h"
#include "Logging.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

Config* Config::m_instance = nullptr;
std::mutex Config::m_mutex;

Config::Config()
{

}

Config* Config::getInstance()//thread safe
{
	if(m_instance == nullptr)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		if(m_instance == nullptr)
		{
			m_instance = new Config();
			static Recovery recov;    //���ڵ������Զ���Դ����
		}
	}
    
    return m_instance;
}

//װ�������ļ�
bool Config::load(const std::string& file_path)
{
    bool ret = true;
    std::fstream file(file_path);
    if(!file.is_open())
        return false;
    std::string line = "";
    while(getline(file, line))
    {
        if((line[0] == '#') || (line[0] == '['))
            continue;
        std::string::size_type pos = line.find("=");
        if(pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos - 1);
        std::string value = line.substr(pos + 2);
        //std::cout << key << std::endl;
        //std::cout << value << std::endl;
        configs.push_back(new ConfigItem(key, value));
    }
    file.close();

   // std::cout << configs.size() << std::endl;

    return ret;
}

//��ȡ�������͵�������Ϣ
int Config::getNum(const std::string& item_name)
{
    int ret = 0;
    const std::string str = getStr(item_name);
    ret = atoi(str.c_str());
    return ret;
}

//��ȡ�ַ�������������Ϣ
const std::string Config::getStr(const std::string& item_name)
{
    std::string ret = "";
    for(size_t i = 0; i < configs.size(); ++i)
    {
        if(configs[i]->ItemName == item_name)
            return configs[i]->ItemContent;
    }
    return ret;
}

Config::~Config()
{
	for(size_t i = 0; i < configs.size(); ++i)
	{
		delete configs[i];
	}
}

