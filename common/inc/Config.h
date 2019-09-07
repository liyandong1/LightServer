#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <mutex>
#include <vector>
#include <string>

//�����ļ���Ŀ
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

//�Ե������ȡ�����ļ�
class Config
{
private:
    static std::mutex m_mutex;
    static Config* m_instance;
    Config();

    //���ڵ�������Դ����
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
    //�����ļ�
    std::vector<ConfigItem*> configs;
    
    static Config* getInstance();

    //װ�������ļ�
    bool load(const std::string& file_path);

    //��ȡ�������͵�������Ϣ
    int getNum(const std::string& item_name);

    //��ȡ�ַ�������������Ϣ
    const std::string getStr(const std::string& item_name);

    ~Config();
};


#endif
