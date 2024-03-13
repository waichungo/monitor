
#include "main.h"

int main(int, char **)
{
    initCPUCounter();
    std::cout << "Hello, from monitor!\n";
    auto inf = Information::getInformation();
    auto infostr=Information2JSON(inf);
    std::cout<<infostr<<"\n";
}
Information Information::getInformation()
{

    Information info;
    info.arch = IsSystemX64() ? "X64" : "x86";
    info.compileTime = getCompileTime();
    info.windowsVersion = getWindowsVersion();

    info.cpu = GetCpuInfo();

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    info.cpuCores = sysinfo.dwNumberOfProcessors;

    info.cpuInfo = getWmicResult("cpu");
    info.osInfo = getWmicResult("os");
    info.driveInfo = getWmicResult("diskdrive");

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    info.totalRam = memInfo.ullTotalPhys;
    info.freeRam = memInfo.ullAvailPhys;

    info.cpuPercentage = getCpuPercentage();
    info.disks = getDisks();

    info.mainBinary = GetExecutable();
    info.hostBinary = getExecutingBinary();

    info.userName = getUsername();
    info.isInteractive = IsUserInteractive();
    info.localIp = getLocalIP();

    auto res = GetBytesFromURL(OBFUSCATED("https://api.ipify.org"));
    if (res.data.size() > 0)
    {
        info.remoteIp.resize(res.data.size());
        memcpy((void *)&info.remoteIp[0], (void *)&res.data[0], res.data.size());
    }
    info.machine = getMachineName();
    info.machineId = GetMachineID();
    info.systemTime = getSystemTime();
    info.screen = getScreenDimensions();

   
    //     info.cpuPercentage = 0;
    return info;
}
Information::Information() : arch(""), compileTime(""), cpu(""), cpuCores(0), cpuInfo({}), cpuPercentage(0), disks(std::vector<Disk>()), dotnetVersions(std::vector<std::string>()), driveInfo({}), freeRam(0), hostBinary(""), isInteractive(false), localIp(""), machine(""), machineId(), mainBinary(""), osInfo({}), remoteIp(""), screen(Screen{}), systemTime(0), totalRam(0), userName(""), windowsVersion(WindowsVersion{})
{
}
