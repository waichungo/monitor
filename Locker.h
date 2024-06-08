#pragma once

#include <string>
#include <mutex>
#include <memory>

class Locker
{
private:
    std::string _name;
    bool _global;
    HANDLE handle;
    bool locked;
    std::shared_ptr<std::mutex> _lck;

public:
    Locker(std::string name, bool global = false, bool unique = true);
    ~Locker();
    bool Lock();
    void UnLock();
};
