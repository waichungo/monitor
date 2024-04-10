#include "Locker.h"
#include "Encrypter.h"
#include "MetaString.h"
#include <unordered_map>
// std::mutex Locker::_lck = std::mutex();
Locker::Locker(std::string name, bool global, bool unique) : locked(false)
{

    this->_global = global;

    this->handle = INVALID_HANDLE_VALUE;

    if (unique)
    {
        name += GetMachineID();
    }
    name = Encrypter::Encrypt(name);
    this->_name = name;

    if (global)
    {

        std::string pref = OBFUSCATED("Global ");
        pref.append(_name);
        this->_name = pref;
    }
    if (this->_name.length() > MAX_PATH)
    {
        this->_name = HashString(this->_name);
    }
}
bool Locker::Lock()
{
    _lck.lock();
    if (locked)
    {
        _lck.unlock();
        return true;
    }
    if (this->handle != INVALID_HANDLE_VALUE && this->handle != NULL)
    {
        CloseHandle(this->handle);
        this->handle = NULL;
        locked = false;
    }
    auto handle = CreateMutexA(NULL, TRUE, _name.c_str());
    auto ret = GetLastError();
    if (ret != ERROR_ALREADY_EXISTS && handle != INVALID_HANDLE_VALUE && handle != NULL)
    {
        this->handle = handle;
        locked = true;
        _lck.unlock();
        return true;
    }
    if (handle != NULL && handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle);
    }
    _lck.unlock();
    return false;
}
void Locker::UnLock()
{
    _lck.lock();
    if (this->handle != NULL && this->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->handle);
        this->handle = INVALID_HANDLE_VALUE;
    }
    locked = false;
    _lck.unlock();
}
Locker::~Locker()
{
    this->UnLock();
}