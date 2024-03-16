#pragma once
#include "Utils.h"
#include <string>
#include <mutex>
// A(const A&) = default;
// A& operator=(const A&) = default;
class Locker
{
private:
    std::string _name;
    bool _global;
    HANDLE handle;
    bool locked;
    std::mutex _lck;
public:
    Locker(std::string name, bool global = false,bool unique=true);
    ~Locker();
    bool Lock();
    void UnLock();

};

