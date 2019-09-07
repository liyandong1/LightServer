#ifndef _MESSAGEFORMAT_H_
#define _MESSAGEFORMAT_H_

#pragma pack(4)

enum CMD
{
	CMD_REGISTER,
	CMD_REGISTER_RESULT,
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_CHATMESSAGE,
    CMD_HEART
};

//消息头
struct DataHeader
{
    int dataLength;//数据长度
    CMD cmd; //具体是什么消息
};

struct Register : public DataHeader
{
	Register()
    {
        dataLength = sizeof(Register) - sizeof(DataHeader);
        cmd = CMD_REGISTER;
        memset(userName, 0, sizeof(userName));
        memset(passWord, 0, sizeof(passWord));
    }
    char userName[32];
    char passWord[32];
};

struct RegisterResult : public DataHeader
{
	RegisterResult()
    {
        dataLength = sizeof(Register) - sizeof(DataHeader);
        cmd = CMD_REGISTER_RESULT;
    }
    bool success;
};

struct Login : public DataHeader
{
    Login()
    {
        dataLength = sizeof(Login) - sizeof(DataHeader);
        cmd = CMD_LOGIN;
        memset(userName, 0, sizeof(userName));
        memset(passWord, 0, sizeof(passWord));
    }
    char userName[32];
    char passWord[32];
};

//通过继承可以不用每次把DataHeader写一遍
struct LoginResult : public DataHeader
{
    LoginResult()
    {
        dataLength = sizeof(LoginResult) - sizeof(DataHeader);
        cmd = CMD_LOGIN_RESULT;
    }
    bool result;
    char info[32];//登录成功也可以返回信息
};

struct LoginOut : public DataHeader
{
    LoginOut()
    {
        dataLength = sizeof(LoginOut) - sizeof(DataHeader);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct LoginOutResult : public DataHeader
{
    LoginOutResult()
    {
        dataLength = sizeof(LoginOutResult) - sizeof(DataHeader);
        cmd = CMD_LOGOUT_RESULT;
    }
    bool result;
    char info[32];//登录成功也可以返回信息
};

struct ChatMessage : public DataHeader
{
    ChatMessage()
    {
        dataLength = sizeof(ChatMessage) - sizeof(DataHeader);
        cmd = CMD_CHATMESSAGE;
    }
    char userName[32];
    char message[128];
};

struct Heart : public DataHeader
{
	Heart()
	{
		dataLength = sizeof(Heart) - sizeof(DataHeader);
        cmd = CMD_HEART;
	}
};

#pragma pack()

#endif // MESSAGEFORMAT_H

