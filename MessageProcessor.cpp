#include "MessageProcessor.h"
#include "MessageHandler.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
using json = nlohmann::json;
void HandleExecute(Message &msg);
void HandleProcess(Message &msg);
void HandleFiles(Message &msg);
void HandleIdle(Message &msg);
void HandleInfo(Message &msg);
void HandleFileOpen(Message &msg);
void HandleFileOperation(Message &msg);
void _processMessage(Message &message);

void MessageProcessor::processMessage(Message message)
{
    std::thread th([=]()
                   { _processMessage(message); });
    th.detach();
}
void HandleExecute(Message &msg)
{
    auto payload = json::parse(msg.data);
    if (payload.contains("file"))
    {
        auto exe = payload["file"].get<std::string>();
        auto args = payload.contains("args") ? payload["args"].get<std::string>() : "";
        auto procRes = StartProcess(exe, args);
        json res;
        res["exitCode"] = procRes.exitcode;
        res["result"] = procRes.result;
        defaultMessageHandler.sendData(msg.sender, MC_EXECUTE, res.dump());
    }
}
void HandleFileOperation(Message &msg)
{
    auto payload = json::parse(msg.data);
    if (payload.contains("file") && payload.contains("move") && payload.contains("dir"))
    {
        auto file = payload["file"].get<std::string>();
        auto dir = payload["dir"].get<std::string>();
        try
        {
            if (!fs::exists(dir))
            {
                fs::create_directories(dir);
            }
        }
        catch (const std::exception &e)
        {
        }

        auto move = payload["move"].get<bool>();
        std::string target = (fs::path(dir) /= fs::path(file).filename()).string();
        if (move)
        {
            MoveFileA(file.c_str(), target.c_str());
        }
        else if (move)
        {
            CopyFileA(file.c_str(), target.c_str(),TRUE);
        }
    }
}
void HandleProcess(Message &msg)
{
    auto procs = GetProcs();
    auto arr = json::array();

    for (auto &proc : procs)
    {
        json item;
        item["pid"] = proc.Pid;
        item["path"] = proc.FullPath;
        item["x86"] = proc.isX86;
        item["memory"] = proc.MemoryUsed;
        item["name"] = proc.Name;
        item["ppid"] = proc.PPID;
        arr.push_back(item);
    }
    defaultMessageHandler.sendData(msg.sender, MC_PROCESS, arr.dump());
}
void HandleFileOpen(Message &msg)
{
    if (fs::exists(msg.data))
    {
        ShellExecuteA(NULL, "open", msg.data.c_str(), NULL, NULL, 0);
    }
}
void _processMessage(Message msg)
{
    try
    {
        if (msg.code == MC_EXECUTE)
        {
            HandleExecute(msg);
        }
        else if (msg.code == MC_PROCESS)
        {
            HandleProcess(msg);
        }
        else if (msg.code == MC_FILES)
        {
            HandleFiles(msg);
        }
        else if (msg.code == MC_INFO)
        {
            HandleInfo(msg);
        }
        else if (msg.code == MC_IDLE)
        {
            HandleIdle(msg);
        }
        else if (msg.code == MC_FILE_OPEN)
        {
            HandleFileOpen(msg);
        }
        else if (msg.code == MC_FILE_DELETE)
        {
            if (fs::exists(msg.data))
            {
                fs::remove(msg.data);
            }
        }
        else if (msg.code == MC_KILLPROCESS)
        {
            if (isNumber(msg.data))
            {
                auto pid = atoi(msg.data.c_str());
                KillProcessByPID(pid);
            }
            else
            {
                KillProcessByName(msg.data);
            }
        }
        else if (msg.code == MC_FILE_OP)
        {
            HandleFileOperation(msg);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
void HandleFiles(Message &msg)
{
    if (fs::exists(msg.data))
    {
        auto arr = json::array();

        for (auto file : fs::directory_iterator(msg.data))
        {
            json item;
            bool isDir = file.is_directory();
            item["name"] = file.path().string();
            item["isDir"] = file.is_directory();
            item["size"] = isDir ? 0 : GetFileSize(file.path().string());
            item["modified"] = std::chrono::duration_cast<std::chrono::seconds>(file.last_write_time().time_since_epoch()).count();
            arr.push_back(item);
        }
        defaultMessageHandler.sendData(msg.sender, MC_FILES, arr.dump());
    }
}
void HandleInfo(Message &msg)
{
    auto info = Information::getInformation();
    auto data = Information2JSON(info);
    defaultMessageHandler.sendData(msg.sender, MC_INFO, data);
}
void HandleIdle(Message &msg)
{
    auto idle = getIdleTime();
    auto data = std::to_string(idle);
    defaultMessageHandler.sendData(msg.sender, MC_IDLE, data);
}