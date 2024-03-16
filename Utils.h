#pragma once
#undef _HAS_STD_BYTE
#include <Windows.h>
#include <filesystem>
#include <time.h>
#include <memory>
#include <windows.h>
#include <tchar.h>
#include <random>
#include <aclapi.h>

#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <chrono>
#include "stringutils.h"
#include <sstream>
#include <vector>
#include "SHA256.h"


#include "MetaString.h"

#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
HANDLE GetDLLHandle();
using namespace andrivet::ADVobfuscator;
const double APPVERSION = 1.0;
typedef char *(*ListFiles)(const char *dir,BOOL onlyDirs, BOOL recursive);
typedef void(*FreeMem)(void*);

namespace fs = std::filesystem;

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#define MAX_DRIVES 256
using std::string;
using std::stringstream;
class AutoReleaseModuleBuffer
{
public:
    AutoReleaseModuleBuffer(LPCTSTR szDllPath) : m_pBuffer(NULL), m_hFileMapping(NULL), m_hFile(NULL), size(0)
    {
        // Open the module and read it into memory buffer
        m_hFile = ::CreateFile(szDllPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
        if (INVALID_HANDLE_VALUE == m_hFile || NULL == m_hFile)
        {

            return;
        }

        this->size = ::GetFileSize(m_hFile, nullptr);

        // Check file size
        DWORD dwFileSize = this->size;
        if (INVALID_FILE_SIZE == dwFileSize || dwFileSize < sizeof(IMAGE_DOS_HEADER))
        {
            ::CloseHandle(m_hFile);
            m_hFile = NULL;

            return;
        }

        m_hFileMapping = ::CreateFileMappingW(m_hFile, 0, PAGE_READONLY, 0, 0, NULL);
        if (NULL == m_hFileMapping)
        {
            ::CloseHandle(m_hFile);
            m_hFile = NULL;

            return;
        }

        m_pBuffer = ::MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0);
        if (NULL == m_pBuffer)
        {
            ::CloseHandle(m_hFileMapping);
            ::CloseHandle(m_hFile);
            m_hFileMapping = NULL;
            m_hFile = NULL;
        }
    }

    ~AutoReleaseModuleBuffer() { Release(); }

    void
    Release()
    {
        if (m_pBuffer)
        {
            ::UnmapViewOfFile(m_pBuffer);
            m_pBuffer = NULL;
        }

        if (m_hFileMapping)
        {
            ::CloseHandle(m_hFileMapping);
            m_hFileMapping = NULL;
        }

        if (m_hFile)
        {
            ::CloseHandle(m_hFile);
            m_hFile = NULL;
        }
    }

    operator LPVOID() { return m_pBuffer; }
    uint64_t size;

private:
    LPVOID m_pBuffer;
    HANDLE m_hFile;
    HANDLE m_hFileMapping;
};
class PEINFO
{
public:
    PEINFO() : isx86(false), isDotNet(false), isDll(false), isConsole(false), peSize(0), fileSize(0), isValid(false),timestamp(0)
    {
    }
    bool isValid;
    bool isx86;
    bool isDotNet;
    bool isDll;
    long timestamp;
    bool isConsole;
    size_t peSize;
    size_t fileSize;
};

std::time_t ParseGitTime(std::string timeString);
PEINFO GetPEInfo(void *buffer, size_t size);
void KillProcessByName(string process);
void KillProcessByPID(int pid);
void KillAllFromDirectory(string dir);

PEINFO GetPEInfo(std::string exe);
class Drive
{
private:
    bool removable;
    bool writable;
    std::string name;

public:
    Drive(std::string name, bool removable) : name(name), writable(false), removable(removable)
    {
        DWORD filesystemFlags = 0;
        const HRESULT result = GetVolumeInformationA(name.c_str(), NULL, 0, NULL, NULL, &filesystemFlags, NULL, 0);
        writable = true;
        if (SUCCEEDED(result))
        {
            if (filesystemFlags & FILE_READ_ONLY_VOLUME)
            {
                writable = false;
            }
        }
    }
    bool isRemovable()
    {
        return this->removable;
    }
    bool isWritable()
    {
        return this->writable;
    }
    std::string getName()
    {
        return this->name;
    }
};

HMODULE GetCurrentModule();

typedef struct Proc
{
    int Pid;
    std::string Name;
    std::string FullPath;
    int PPID;
    long MemoryUsed;
    bool isX86;

} Proc;
typedef struct ExecResult
{
    int exitcode;
    int PID;
    std::string result;
} ExecResult;

bool IsCurrentProcessDll();
bool ExtractArchive(std::string src,std::string dest);
bool VerifyArchiveExtracted(std::string src,std::string dest);
// void initializeZipLib();

std::time_t to_time_t_type(std::filesystem::file_time_type tp);
std::string GetProcessCommandLine(int pid);
uint64_t GetRandom(uint64_t min = 1, uint64_t max = 100000000);
bool IsCurrentProcessX86();
std::string HashString(std::string &str);
ExecResult StartProcess(std::string file, std::string cmd,int timeoutInSecs=0,bool *shouldExit=nullptr);

bool IsValidModule(vector<uint8_t> &buffer);

bool IsAdmin();
bool ProcessIsRunning(std::string file);
std::vector<std::string> GetAntivirusProducts();

void RunApp(std::string file);
bool writeFile(std::string file,std::vector<uint8_t> data);
bool writeFile(std::string file,std::string data);
bool writeFile(std::string file, void *data, size_t size);

std::string GetInstallDir();
std::string generate_uuid_v4();
BOOL IsService();
bool IsValidModule(vector<uint8_t> &buffer);
bool IsValidModule(void *buffer, uint64_t size);
BOOL IsUserInteractive();

void RunApps();

std::string GetUsersHome();
std::string GetExecutable();
std::vector<Proc> GetProcs();
bool IsValidPE(std::string exe);
bool ChangeSubsystem(std::string exe, bool console);

std::string GetTempFileOrDirName(bool isDirectory = false, std::string suffix = "tmp");

bool IsX86Process(HANDLE process);
bool IsX86ProcessFromPID(int pid);

bool IsX86FromBuffer(std::vector<uint8_t> &buffer);
bool IsX86FromBuffer(void *buffer, uint64_t size);

bool IsX64(const char *file);
bool IsX86(const char *file);
bool Exists(std::string path);
int getppid(int pid);
std::vector<uint8_t> ReadFile(const char *filename, bool *success = nullptr);
// bool IsConsole();
// bool CheckConsole(std::string exe);
std::string RemoveExt(std::string file);

bool IsSystemX86();
bool IsSystemX64();

std::wstring ToWide(std::string datastring);

std::string ReplaceInvalidFileChars(std::string path);
bool HaveFolderPermisions(std::string exe);
uint64_t GetFileSize(std::string file);
std::string generate_uuid_v4();
// bool ExtractZipFileToBuffer(std::string file, std::vector<unsigned char> *buffer, std::string pass = "");
// bool ExtractZipFile(std::string file, std::string destDirectory, std::string pass = "");
// bool ExtractZipToBuffer(std::stringstream input, std::vector<unsigned char> *buffer, std::string pass = "");

// bool CreateZip(std::string sourcefile, std::string destFile, std::string pass = "");
// bool CreateZipFilesInMemory(std::vector<unsigned char> *buffer, std::vector<std::string> files, std::string pass = "");
// bool CreateZipInMemory(std::vector<unsigned char> *buffer, std::string entryname, std::string sourceBuffer, std::string pass = "");
// bool CreateZipFileInMemory(std::vector<unsigned char> *buffer, std::string file, std::string pass = "");

std::string GetStringValueFromHKLM(const std::string &regSubKey, const std::string &regValue);
std::string GetInstallDir();
std::string GetMac();
std::string GetMachineID();
std::vector<Drive> GetDrives();