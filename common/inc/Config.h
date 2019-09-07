#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <mutex>
#include <vector>
#include <string>

//配置文件条目
struct ConfigItem
{
    std::string ItemName;
    std::string ItemContent;

    ConfigItem(const std::string& str1, const std::string& str2)
    {
        this->ItemName = str1;
        this->ItemContent = str2;
    }
};

//以单例类读取配置文件
class Config
{
private:
    static std::mutex m_mutex;
    static Config* m_instance;
    Config();

    //用于单例类资源回收
    class Recovery
    {
    public:
        ~Recovery()
        {
            if(m_instance != nullptr)
            {
                delete m_instance;
            }
        }
    };

public:
    //配置文件
    std::vector<ConfigItem*> configs;
    
    static Config* getInstance();

    //装载配置文件
    bool load(const std::string& file_path);

    //获取数字类型的配置信息
    int getNum(const std::string& item_name);

    //获取字符串类型配置信息
    const std::string getStr(const std::string& item_name);

    ~Config();
};


#endif
