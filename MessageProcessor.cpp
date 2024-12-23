#include "MessageProcessor.h"
#include "MessageHandler.h"
#include <nlohmann/json.hpp>
#include <thread>
#include "GZip.h"
#include <chrono>
#include "ZipExt.h"
#include "Base64.hpp"
#include "Models.h"
#include "HttpUtil.h"
#include "DB.h"
using json = nlohmann::json;
void HandleExecute(Message &msg);
void HandleProcess(Message &msg);
void HandleFiles(Message &msg);
void HandleIdle(Message &msg);
void HandleApps(Message &msg);
void HandleInfo(Message &msg);
void HandleFileOpen(Message &msg);
void HandleFileOperation(Message &msg);
void HandleThumbnail(Message &msg);
void HandleCapture(Message &msg);
void HandleDataPart(Message &msg);
void HandleRunnableStartUp(Message &msg);
void HandleRunnableUninstall(Message &msg);
void _processMessage(Message &message);
std::string runnablesToJSON();

void MessageProcessor::processMessage(Message message)
{
    std::thread th([=]()
                   { 
                    Message msg=message;
                    _processMessage(msg); });
    th.detach();
}
DataPartManager::DataPartManager() {}
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
        res["command"] = exe + " " + args;
        res["time"] = getSystemTime();
        res["result"] = procRes.result;
        defaultMessageHandler->sendData(msg.sender, MC_EXECUTE, compressGzip(res.dump()), "", true);
    }
}
void EnqueueDataPart(DataPartManager &part, std::string recepient, int startIndex, bool compress, int partSize)
{
    part.lastInteraction = getSystemTime();

    if (startIndex >= part.data.size())
    {
        bool sent = defaultMessageHandler->sendData(recepient, MC_DATA_PART_COMPLETE, part.id);
        return;
    }
    std::vector<uint8_t> dataPart;

    if ((startIndex + partSize) > part.data.size())
    {
        partSize = part.data.size() - startIndex;
    }
    if (partSize <= 0)
        return;
    dataPart.resize(partSize);
    memcpy(&dataPart[0], &part.data[startIndex], partSize);
    if (compress)
    {
        dataPart = compressGzip(dataPart);
    }

    PartMeta meta;
    meta.set_id(part.id);
    meta.set_compress(compress);
    meta.set_internally_compressed(part.compressed);
    meta.set_request_size(partSize);
    meta.set_size(part.data.size());
    meta.set_start(startIndex);
    meta.set_type(part.kind);
    meta.set_arg("");
    meta.set_is_file(false);

    auto metaStr = meta.to_json().dump();
    part.lastInteraction = getSystemTime();
    defaultMessageHandler->sendData(recepient, MC_DATA_PART, dataPart, metaStr, false);
}
void EnqueueFilePart(FilePartManager &part, std::string recepient, int startIndex, bool compress, int partSize)
{

    if (startIndex >= part.size)
    {
        bool sent = defaultMessageHandler->sendData(recepient, MC_DATA_PART_COMPLETE, part.id);
        return;
    }
    if ((startIndex + partSize) > part.size)
    {
        partSize = part.size - startIndex;
    }
    std::vector<uint8_t> dataPart = part.Read(startIndex, partSize);
    if (dataPart.size() == 0)
    {
        return;
    }
    if (compress)
    {
        dataPart = compressGzip(dataPart);
    }

    PartMeta meta;
    meta.set_id(part.id);
    meta.set_compress(compress);
    meta.set_internally_compressed(false);
    meta.set_request_size(partSize);
    meta.set_size(part.size);
    meta.set_start(startIndex);
    meta.set_type(MC_FILE_TRANSFER);
    meta.set_arg(part.path);
    meta.set_is_file(true);

    auto metaStr = meta.to_json().dump();

    defaultMessageHandler->sendData(recepient, MC_DATA_PART, dataPart, metaStr, compress);
}
void HandleDataPart(Message &msg)
{
    bool isValidJson = false;
    try
    {
        json::parse(msg.data);
        isValidJson = true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    if (isValidJson)
    {
        PartMeta meta = PartMeta::from_json(msg.data);
        if (meta.get_type() == MC_CAPTURE)
        {
            dataPartsAccessor.Access([&](std::map<std::string, DataPartManager> *queue)
                                     {
auto dataPartsQueue=*queue;
                if (queue->count(meta.get_id()))
            {
                DataPartManager *mn = &dataPartsQueue[meta.get_id()];
                EnqueueDataPart(*mn, msg.sender, meta.get_start(), meta.get_compress(), meta.get_request_size());
            }
            else
            {
                defaultMessageHandler->sendData(msg.sender, MC_DATA_PART_STALE, meta.get_id());
            } });
        }
        else if (meta.get_type() == MC_FILE_TRANSFER)
        {
            filePartsAccessor.Access([&](std::map<std::string, FilePartManager> *queue)
                                     {
auto filePartsQueue=*queue;
if (filePartsQueue.count(meta.get_id()) == 0)
            {
                FilePartManager mn(meta.get_arg(), meta.get_id());
                filePartsQueue.emplace(meta.get_id(), mn);
            }

            auto ref = &filePartsQueue[meta.get_id()];

            EnqueueFilePart(*ref, msg.sender, meta.get_start(), meta.get_compress(), meta.get_request_size()); });
        }
    }
}
void HandleCapture(Message &msg)
{
    if (IsUserInteractive())
    {
        auto img = CaptureWindowAsJPEG(GetDesktopWindow(), FALSE);
        if (img.size > 0 && img.buffer != NULL)
        {
            auto data = compressGzip(img.buffer, img.size);
            free(img.buffer);
            if (data.size() > 0)
            {
                std::string id = msg.data.size() > 0 ? msg.data : generate_uuid_v4();
                int requestSize = 1024 * 40;

                bool internallyCompressed = true;
                DataPartManager mn(data, MC_CAPTURE, internallyCompressed, id);

                dataPartsAccessor.Access([&](std::map<std::string, DataPartManager> *queue)
                                         {
            (*queue)[id]= mn;
auto dataPartsQueue=*queue;

            EnqueueDataPart(mn, msg.sender, 0, false, requestSize); });

                // bool sent = defaultMessageHandler->sendData(msg.sender, MC_CAPTURE, data, true);
                // std::cout << sent ? ">>> Sent capture\n" : ">>> Failed to send capture";
            }
        }
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
            CopyFileA(file.c_str(), target.c_str(), TRUE);
        }
    }
}
std::string runnablesToJSON()
{
    auto runnables = findRunnables({});
    auto arr = json::array();

    for (auto &runnable : runnables)
    {
        json item;
        app::to_json(item, runnable);
        arr.push_back(item);
    }
    return arr.dump();
}
void HandleProcess(Message &msg)
{
    defaultMessageHandler->sendData(msg.sender, MC_PROCESS, compressGzip(processesToJSON()), "", true);
}
void HandleApps(Message &msg)
{
    defaultMessageHandler->sendData(msg.sender, MC_APPS, runnablesToJSON(), "");
}
void HandleRunnableStartUp(Message &msg)
{
    if (!msg.data.empty() && (msg.code == MC_START_APP || msg.code == MC_STOP_APP))
    {
        std::map<std::string, DBValue> params;

        DBValue q2;
        q2.equality = DBEquality::EQUAL;
        q2.stringValue = std::make_shared<std::string>(msg.data);
        params.emplace("remoteID", q2);

        auto runnables = findRunnables(params, 1);
        if (!runnables.empty())
        {
            auto runnable = runnables[0];
            bool start = msg.code == MC_START_APP;
            if (runnable.run != start)
            {
                runnable.run = start;
                if (!start && !runnable.mainExe.empty())
                {
                    KillAllFromDirectory(fs::path(runnable.mainExe).parent_path().string());
                }
                UpdateRunnable(runnable);
            }
        }
    }
}
void HandleRunnableUninstall(Message &msg)
{
    if (!msg.data.empty() && msg.code == MC_UNINSTALL_APP)
    {
        std::map<std::string, DBValue> params;

        DBValue q2;
        q2.equality = DBEquality::EQUAL;
        q2.stringValue = std::make_shared<std::string>(msg.data);
        params.emplace("remoteID", q2);

        auto runnables = findRunnables(params, 1);
        if (!runnables.empty())
        {
            auto runnable = runnables[0];

            if (runnable.run)
            {
                runnable.run = false;
                for (size_t i = 0; i < 20; i++)
                {
                    try
                    {
                        UpdateRunnable(runnable);
                        break;
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                    Sleep(200);
                }
            }
            Sleep(200);

            auto dir = runnable.mainExe.empty() ? (fs::path(GetAppsDir()) /= runnable.remoteID).string() : fs::path(runnable.mainExe).parent_path().string();
            KillAllFromDirectory(dir);
            for (size_t i = 0; i < 20; i++)
            {
                try
                {
                    fs::remove_all(dir);
                    break;
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
                Sleep(200);
            }
            for (size_t i = 0; i < 20; i++)
            {
                try
                {
                    deleteRunnable(runnable.id);
                    break;
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
                Sleep(200);
            }
            
        }
    }
}
void HandleFileUpload(Message &msg)
{
    if (!msg.data.empty() && msg.code == MC_FILE_UPLOAD)
    {
        std::string driveId = "";
        std::string file = "";
        DriveKind kind = DriveKind::DR_UNDEFINED;
        try
        {
            json j = json::parse(msg.data);
            if (j.contains("driveId"))
            {
                driveId = j.at("driveId").get<std::string>();
            }
            if (j.contains("file"))
            {
                file = j.at("file").get<std::string>();
            }
            if (j.contains("kind"))
            {
                auto type = j.at("kind").get<int>();
                if (type == DriveKind::DR_APPWRITE)
                {
                    kind = DriveKind::DR_APPWRITE;
                }
                else if (type == DriveKind::DR_GOOGLE)
                {
                    kind = DriveKind::DR_GOOGLE;
                }
            }
        }
        catch (const std::exception &e)
        {
            if (Exists(msg.data))
            {
                file = msg.data;
            }
            std::cerr << e.what() << '\n';
        }

        if (!file.empty() && Exists(file))
        {
            try
            {
                if (driveId.empty() || kind == DR_UNDEFINED)
                {
                    for (size_t i = 0; i < 5; i++)
                    {
                        WaitForConnection();
                        auto drives = fetchGoogleDrives();
                        if (!drives.empty())
                        {
                            kind = DriveKind::DR_GOOGLE;
                            driveId = drives[0].get_id();
                            break;
                        }

                        Sleep(1000);
                    }
                }
                if (driveId.empty() || kind == DR_UNDEFINED)
                {
                    for (size_t i = 0; i < 5; i++)
                    {
                        WaitForConnection();
                        auto appDrives = fetchAppwriteDrives();
                        if (!appDrives.empty())
                        {
                            kind = DriveKind::DR_APPWRITE;
                            driveId = appDrives[0].get_id();
                            break;
                        }
                        Sleep(1000);
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            if (kind != DR_UNDEFINED && !driveId.empty())
            {
                std::map<std::string, DBValue> params;

                DBValue q2;
                q2.equality = DBEquality::EQUAL;
                q2.stringValue = std::make_shared<std::string>(file);
                params.emplace("path", q2);

                auto uploads = findUploads(params, 1);
                if (!uploads.empty())
                {
                    auto upload = uploads[0];
                    if (!upload.completed)
                    {
                        return;
                    }
                }
                app::Upload upload;
                upload.client_id = "";
                upload.completed = false;
                upload.drive_id = driveId;
                upload.stopped = false;
                upload.type = kind;
                upload.path = file;
                upload.id = generate_uuid_v4();

                SaveUpload(upload);
            }
        }
    }
}
void HandleFileOpen(Message &msg)
{
    // if (fs::exists(msg.data))
    // {
    auto fL = ToWide(msg.data);
    ShellExecuteW(NULL, L"open", fL.c_str(), NULL, NULL, 0);
    // }
}
void HandleThumbnail(Message &msg)
{
    if (fs::exists(msg.data))
    {
        auto img = CaptureFileThumbnail(msg.data);
        if (img.size() > 0)
        {
            auto data = compressGzip(img);
            defaultMessageHandler->sendData(msg.sender, MC_THUMBNAIL, data, "", true);
        }
    }
}
void _processMessage(Message &msg)
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
        else if (msg.code == MC_APPS)
        {
            HandleApps(msg);
        }
        else if (msg.code == MC_CAPTURE)
        {
            HandleCapture(msg);
        }
        else if (msg.code == MC_FILES)
        {
            HandleFiles(msg);
        }
        else if (msg.code == MC_DATA_PART)
        {
            HandleDataPart(msg);
        }
        else if (msg.code == MC_START_APP || msg.code == MC_STOP_APP)
        {
            HandleRunnableStartUp(msg);
        }
        else if (msg.code == MC_UNINSTALL_APP)
        {
            HandleRunnableUninstall(msg);
        }
        else if (msg.code == MC_FILE_UPLOAD)
        {
            HandleFileUpload(msg);
        }
        else if (msg.code == MC_INFO)
        {
            HandleInfo(msg);
        }
        else if (msg.code == MC_PING)
        {
            defaultMessageHandler->sendData(msg.sender, MC_PING, "");
        }
        else if (msg.code == MC_IDLE)
        {
            HandleIdle(msg);
        }
        else if (msg.code == MC_THUMBNAIL)
        {
            HandleThumbnail(msg);
        }
        else if (msg.code == MC_LOCK)
        {
            LockWorkStation();
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
        else if (msg.code == MC_SENDEVENTS)
        {
            HandleInfo(msg);
            HandleProcess(msg);
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
    std::string dir = "C:\\";
    if (fs::exists(msg.data))
    {
        dir = msg.data;
    }

    std::string cleanDir = trimString(dir, "\\");
    std::string out;
    out += dir;
    for (auto &info : listFiles(dir))
    {
        out += "\n";
        out += info.path;
        out += "|||" + std::to_string(info.size);
        out += "|||" + std::to_string(info.modified);
        out += "|||" + std::to_string(info.isDir ? 1 : 0);
    }

    defaultMessageHandler->sendData(msg.sender, MC_FILES, compressGzip(out), "", true);
}
void HandleInfo(Message &msg)
{
    auto info = Information::getInformation();
    auto data = Information2JSON(info);
    defaultMessageHandler->sendData(msg.sender, MC_INFO, data);
}
void HandleIdle(Message &msg)
{
    auto idle = getIdleTime();
    auto data = std::to_string(idle);
    defaultMessageHandler->sendData(msg.sender, MC_IDLE, data);
}
SafeDataPartQueueAccessor dataPartsAccessor;
SafeFilePartQueueAccessor filePartsAccessor;

SafeDataPartQueueAccessor::SafeDataPartQueueAccessor() : _lck(std::mutex()), queue(std::map<std::string, DataPartManager>()) {}
void SafeDataPartQueueAccessor::Access(DataMapAccessorFunc fn)
{
    if (fn != nullptr)
    {
        _lck.lock();
        try
        {
            fn(&queue);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        _lck.unlock();
    }
}
size_t SafeDataPartQueueAccessor::size()
{
    _lck.lock();
    size_t size = queue.size();
    _lck.unlock();
    return size;
}
SafeFilePartQueueAccessor::SafeFilePartQueueAccessor() : _lck(std::mutex()), queue(std::map<std::string, FilePartManager>()) {}
void SafeFilePartQueueAccessor::Access(FileMapAccessorFunc fn)
{
    if (fn != nullptr)
    {
        _lck.lock();
        try
        {
            fn(&queue);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        _lck.unlock();
    }
}
size_t SafeFilePartQueueAccessor::size()
{
    _lck.lock();
    size_t size = queue.size();
    _lck.unlock();
    return size;
}
DataPartManager::DataPartManager(std::vector<uint8_t> payload, MessageCode internalType, bool isCompressed, std::string identifier) : id(identifier), data(payload), kind(internalType), compressed(isCompressed)
{
}

FilePartManager::FilePartManager(std::string sourcePath, std::string identifier) : id(identifier), path(sourcePath), lastInteraction(getSystemTime()), size(GetFileSize(sourcePath)), file(NULL)
{
    // file = fopen(path.c_str(), "rb");
}
int64_t FilePartManager::written()
{
    return position;
}
std::vector<uint8_t> FilePartManager::Read(int start, int totalRead)
{
    // _lck.lock();
    std::vector<uint8_t> res;
    if (file == NULL)
    {
        file = fopen(path.c_str(), "rb");
    }
    lastInteraction = getSystemTime();
    if ((start + totalRead > size))
    {
        totalRead = size - start;
    }
    if (totalRead == 0 || start > size)
    {
        // _lck.unlock();
        return res;
    }
    if (start != position)
    {
        int ret = fseek(file, start, SEEK_SET);
        if (ret != 0)
        {
            // _lck.unlock();
            return res;
        }
    }
    res.resize(totalRead);
    int read = fread(&res[0], 1, res.size(), file);
    if (read < res.size())
    {
        res.resize(read);
    }
    position = ftell(file);
    lastInteraction = getSystemTime();
    // _lck.unlock();
    return res;
}
FilePartManager::FilePartManager() {}
FilePartManager::~FilePartManager()
{
    if (file != NULL)
    {
        fclose(file);
        file = NULL;
    }
}
std::mutex _cleanLck;
void cleanDataParts(std::map<std::string, DataPartManager> *queue)
{
    std::vector<std::string> ids;
    ids.reserve(20);
    for (auto it = queue->begin(); it != queue->end(); it++)
    {
        if ((getSystemTime() - it->second.lastInteraction) > 40)
        {
            ids.push_back(it->first);
        }
    }
    if (ids.size() > 0)
    {
        for (auto &id : ids)
        {
            queue->erase(id);
        }
    }
}
void cleanFileParts(std::map<std::string, FilePartManager> *queue)
{
    std::vector<std::string> ids;
    ids.reserve(20);
    for (auto it = queue->begin(); it != queue->end(); it++)
    {
        if ((getSystemTime() - it->second.lastInteraction) > 40)
        {
            ids.push_back(it->first);
        }
    }
    if (ids.size() > 0)
    {
        for (auto &id : ids)
        {
            queue->erase(id);
        }
    }
}
void cleanParts()
{
    while (true)
    {
        if (dataPartsAccessor.size() == 0 && filePartsAccessor.size() == 0)
        {
            Sleep(5000);
        }
        else
        {
            if (dataPartsAccessor.size() > 0)
            {
                dataPartsAccessor.Access(cleanDataParts);
            }
            if (filePartsAccessor.size() > 0)
            {
                filePartsAccessor.Access(cleanFileParts);
            }
            Sleep(2000);
        }
    }
}
void partCleanerTask()
{
    if (_cleanLck.try_lock())
    {
        std::thread th([&]()
                       { cleanParts(); });
        th.detach();
    }
}