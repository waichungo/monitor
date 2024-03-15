
#include "main.h"
int64_t lastInfoQuery = 0;
Information lastInfo;
void trays();
int main(int, char **)
{
    // trays();
    Start();
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    std::vector<HWND> *windowList = reinterpret_cast<std::vector<HWND> *>(lParam);
    // if (IsWindowVisible(hwnd))
    { // Check if window is visible
        windowList->push_back(hwnd);
    }
    return TRUE; // Continue enumeration
}
void trays()
{
    std::vector<HWND> windowHandles;

    // Enumerate all top-level windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowHandles));

    std::cout << "Processes in the system tray:" << std::endl;

    // Iterate through the list of window handles
    const int pid = 14720;
    for (const HWND &hwnd : windowHandles)
    {
        char windowTitle[256];
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle));
        std::string titleStr = windowTitle;
        BOOL visible = IsWindowVisible(hwnd);
        if (StringUtils::startsWith(titleStr, "DeskPi"))
        {
        }
        DWORD ppid = 0;
        GetWindowThreadProcessId(hwnd, &ppid);
        if (ppid == pid)
        {
            std::cout << "Found\n";
        }
    }
}
int64_t getIdleTime()
{
    LASTINPUTINFO info{0,0};
    info.cbSize=sizeof(LASTINPUTINFO);

    BOOL res = GetLastInputInfo(&info);
    return (int)((GetTickCount() - info.dwTime)/1000);
}
void Start()
{
    initCPUCounter();
    std::cout << "Hello, from monitor!\n";
    std::chrono::high_resolution_clock clk;

    for (;;)
    {
        auto start = clk.now();
        auto inf = Information::getInformation();
        auto now = clk.now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        auto infostr = Information2JSON(inf);
        // std::cout << infostr << "\n";
        std::cout << elapsed << "\n";
        Sleep(5000);
    }
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

    bool starting = lastInfoQuery == 0;
    std::thread th1([&]()
                    { info.cpuInfo = starting || lastInfo.cpuInfo.size() == 0 ? getWmicResult("cpu") : lastInfo.cpuInfo; });
    std::thread th2([&]()
                    { info.osInfo = starting || lastInfo.cpuInfo.size() == 0 ? getWmicResult("os") : lastInfo.osInfo; });
    std::thread th3([&]()
                    { info.driveInfo = starting || lastInfo.cpuInfo.size() == 0 ? getWmicResult("diskdrive") : lastInfo.driveInfo; });
    th1.join();
    th2.join();
    th3.join();

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    info.totalRam = memInfo.ullTotalPhys;
    info.freeRam = memInfo.ullAvailPhys;

    info.cpuPercentage = getCpuPercentage();

    info.disks = starting || lastInfo.cpuInfo.size() == 0 ? getDisks() : lastInfo.disks;
    info.mainBinary = starting || lastInfo.cpuInfo.size() == 0 ? GetExecutable() : lastInfo.mainBinary;
    info.hostBinary = starting || lastInfo.cpuInfo.size() == 0 ? getExecutingBinary() : lastInfo.hostBinary;
    info.screen = starting || lastInfo.cpuInfo.size() == 0 ? getScreenDimensions() : lastInfo.screen;
    info.isInteractive = IsUserInteractive();
    info.userName = getUsername();
    info.machine = getMachineName();
    info.machineId = GetMachineID();

    if (lastInfo.localIp == "" || (getSystemTime() - lastInfoQuery) > 60)
    {
        info.localIp = getLocalIP();
    }
    else
    {
        info.localIp = lastInfo.localIp;
    }

    if (lastInfo.remoteIp == "" || (getSystemTime() - lastInfoQuery) > 60)
    {
        auto res = GetBytesFromURL(OBFUSCATED("https://api.ipify.org"));
        if (res.data.size() > 0)
        {
            info.remoteIp.resize(res.data.size());
            memcpy((void *)&info.remoteIp[0], (void *)&res.data[0], res.data.size());
        }
    }
    else
    {
        info.remoteIp = lastInfo.remoteIp;
    }
    info.idleTime=getIdleTime();

    info.systemTime = getSystemTime();
    lastInfoQuery = getSystemTime();
    lastInfo = info;

    //     info.cpuPercentage = 0;
    return info;
}
Information::Information() : arch(""), compileTime(""), cpu(""), cpuCores(0), cpuInfo({}), cpuPercentage(0), disks(std::vector<Disk>()), dotnetVersions(std::vector<std::string>()), driveInfo({}), freeRam(0), hostBinary(""), isInteractive(false), localIp(""), machine(""), machineId(), mainBinary(""), osInfo({}), remoteIp(""), screen(Screen{}), systemTime(0), totalRam(0), userName(""), windowsVersion(WindowsVersion{}),idleTime(0)
{
}
