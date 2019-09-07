//@Athor LiYanDong
//@Email 416620418@qq.com
#pragma once

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator= (const noncopyable&);
};
