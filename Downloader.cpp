#include "Downloader.h"
#include "Models.h"
#include "Utils.h"
#include "Locker.h"
#include "HttpUtil.h"
#include "DB.h"

#include "App.h"
#include "Base64.hpp"
#include <curl/curl.h>

std::string USERAGENT = OBFUSCATED("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4573.0 Safari/537.36");
size_t FileWriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    FileBag *bag = (FileBag *)data;
    size_t realsize = size * nmemb;

    if (bag->stopSignal != nullptr && *bag->stopSignal)
    {
        return CURL_WRITEFUNC_PAUSE;
    }
    auto written = fwrite(ptr, 1, realsize, bag->file);
    if (bag->progressData != nullptr)
    {
        if (realsize > 0)
        {
            bag->progressData->transferred = bag->progressData->transferred + written;
        }
    }

    return written;
}
std::shared_ptr<app::DownloadProgress> createOrFindDownloadProgressForDownload(app::Download &download)
{
    std::shared_ptr<app::DownloadProgress> prog;
    DBValue val;

    val.stringValue = std::make_shared<std::string>(download.get_id());
    std::map<std::string, DBValue> mp;
    mp.emplace("id", val);

    auto progs = findDownloadProgresses(mp, 1);
    if (!progs.empty())
    {
        prog = std::make_shared<app::DownloadProgress>(progs[0]);
    }
    else
    {
        auto tmpProg = app::DownloadProgress();
        tmpProg.complete = false;
        tmpProg.machine_id = GetMachineID();
        tmpProg.status = DownloadStatus::DS_DOWNLOADING;
        tmpProg.download_type = download.type;
        tmpProg.id = download.id;
        tmpProg.resource = download.link;
        tmpProg.doneWithUpdate = false;
        prog = SaveDownloadProgress(tmpProg);
    }
    return prog;
}

std::string GetDefaultDownloadsDir()
{
    auto dir = fs::path(GetAssetDir()) /= OBFUSCATED("downloads");
    if (!Exists(dir.string()))
    {
        try
        {
            fs::create_directories(dir);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    return dir.string();
}
std::string GetDefaultAppsDownloadsDir()
{
    auto dir = fs::path(GetAssetDir()) /= OBFUSCATED("appsdownload");
    if (!Exists(dir.string()))
    {
        try
        {
            fs::create_directories(dir);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    return dir.string();
}
typedef struct ProgressArg
{
    app::Download *download;
    std::atomic<int64_t> *size;
    std::atomic<int64_t> *downloaded;
    std::string name;
    std::atomic_bool *stopped;
    std::atomic_bool *completed;
} ProgressArg;
void downloadProgressCallbackFunction(ProgressArg *payload)
{
    if (payload != nullptr && payload->download != nullptr)
    {
        auto prog = createOrFindDownloadProgressForDownload(*payload->download);
        if (prog)
        {
            std::string progId = prog->id;
            while (payload->stopped != nullptr && !*payload->stopped)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    Sleep(200);
                    if (*payload->stopped)
                    {
                        break;
                    }
                }
                std::shared_ptr<app::DownloadProgress> loadedProg = createOrFindDownloadProgressForDownload(*payload->download);

                if (loadedProg != nullptr && payload->downloaded != nullptr)
                {
                    loadedProg->downloaded = *payload->downloaded;
                    loadedProg->complete = payload->completed && *payload->completed;
                    if (loadedProg->name.empty())
                    {
                        loadedProg->name = payload->name;
                    }
                    if (loadedProg->size == 0)
                    {
                        loadedProg->size = payload->size->load();
                    }
                    try
                    {
                        UpdateDownloadProgress(*loadedProg);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
    }
}
void downloadDownload(app::Download download)
{
    if (download.completed || download.stopped)
        return;
    std::string mtx = OBFUSCATED("Download__") + download.get_id();
    // mtx += OBFUSCATED("downloader_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        WaitForConnection();
        auto info = GetLinkInfo(download.get_link());
        auto target = fs::path(GetDefaultDownloadsDir()) /= info.name;

        std::atomic_bool stopDownload = false;
        std::atomic_bool quitDownloadStatusMonitorThread = false;
        std::atomic_bool stopProgressThread = false;
        std::atomic_bool completed = false;

        std::atomic<int64_t> downloaded = 0;
        std::atomic<int64_t> size = 0;

        std::thread downloadStatusMonitorThread((std::function<void()>)[&]() {
            while (!quitDownloadStatusMonitorThread)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    Sleep(200);
                    if (quitDownloadStatusMonitorThread)
                    {
                        break;
                    }
                }
                auto loadedDl = findDownload(download.local_id);
                auto prog = createOrFindDownloadProgressForDownload(*loadedDl);
                if (loadedDl)
                {
                    if (loadedDl->stopped)
                    {
                        stopDownload.store(true);

                        if (prog)
                        {
                            prog->status = DownloadStatus::DS_STOPPED;
                            SaveDownloadProgress(*prog);
                        }
                        break;
                    }
                    else
                    {
                    }
                }
            }
        });

        ProgressArg arg;

        arg.stopped = &stopProgressThread;
        arg.downloaded = &downloaded;
        arg.completed = &completed;
        arg.size = &size;
        arg.download = &download;

        std::thread progressThread(downloadProgressCallbackFunction, &arg);

        auto res = DownloadFile(target.string(), download.link, {}, &stopDownload, [&](ProgressData progData)
                                {
                                    arg.size->store(progData.size);
                                    arg.downloaded->store(progData.transferred);
                                    if(!progData.name.empty()){
                                    arg.name=progData.name; 
                                    } });
        completed = !stopDownload && res.success;
        quitDownloadStatusMonitorThread = true;
        stopProgressThread = true;
        Sleep(200);
        if (progressThread.joinable())
        {
            progressThread.join();
        }
        downloadStatusMonitorThread.join();
        if (res.success)
        {
            auto dl = findDownload(download.local_id);
            if (dl != nullptr)
            {
                dl->completed = completed;
                auto prog = createOrFindDownloadProgressForDownload(download);
                if (prog)
                {
                    prog->complete = completed;
                    prog->status = completed ? DownloadStatus::DS_COMPLETE : DownloadStatus::DS_STOPPED;
                    UpdateDownloadProgress(*prog);
                }
                UpdateDownload(*dl);
            }
        }
        else
        {
            if (!res.error.empty())
            {
                auto status = createOrFindDownloadProgressForDownload(download);
                status->error = res.error;
                SaveDownloadProgress(*status);
            }
            if (stopDownload)
            {
            }
        }
    }
}

void downloadRunnable(Runnable runnable)
{
    if (runnable.downloaded)
        return;
    std::string mtx = OBFUSCATED("Runnable Download__") + runnable.remoteID + runnable.link;
    // mtx += OBFUSCATED("downloader_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        try
        {
            WaitForConnection();
            std::atomic_bool stopDownload = false;

            auto fname = runnable.remoteID;
            auto ext = getFileExtension(runnable.name);
            if (ext.empty() && !runnable.link.empty())
            {
                auto info = GetLinkInfo(runnable.link);
                ext = runnable.link;
            }
            fname += "." + ext;

            auto target = fs::path(GetDefaultAppsDownloadsDir()) /= fname;
            if (runnable.cloudType == DriveKind::DR_UNDEFINED && !runnable.link.empty())
            {
                auto res = DownloadFile(target.string(), runnable.link, {}, &stopDownload);
                // completed = true;
                // progressThread.join();
                if (res.success)
                {
                    auto dl = findRunnable(runnable.id);
                    if (dl != nullptr)
                    {
                        dl->downloaded = true;
                        UpdateRunnable(*dl);
                    }
                }
            }
            else if (runnable.cloudType == DriveKind::DR_GOOGLE && !runnable.drive_id.empty())
            {
                std::string accessToken = GetGDriveToken(runnable.drive_id);
                if (!accessToken.empty())
                {
                    std::map<std::string, std::string> headers;
                    std::string tokenHeaderPart = OBFUSCATED("Bearer ");
                    tokenHeaderPart += accessToken;
                    headers.emplace(OBFUSCATED("Authorization"), tokenHeaderPart);

                    std::string link = OBFUSCATED("https://www.googleapis.com/drive/v3/files/") + runnable.remoteID + "?alt=media";
                    auto res = DownloadFile(target.string(), link, headers, &stopDownload);
                    if (res.success)
                    {
                        auto dl = findRunnable(runnable.id);
                        if (dl != nullptr)
                        {
                            dl->downloaded = true;
                            UpdateRunnable(*dl);
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}
static size_t FilenameExtractCallback(char *buffer, size_t size, size_t nitems, std::string *userdata)
{
    size_t totalSize = size * nitems;
    std::string header(buffer, totalSize);

    std::string prefix = "Content-Disposition: ";
    if (header.compare(0, prefix.size(), prefix) == 0)
    {
        std::size_t filenamePos = header.find("filename=");
        if (filenamePos != std::string::npos)
        {
            std::string filename = header.substr(filenamePos + 9); // 9 is the length of "filename="
            if (!filename.empty() && filename[0] == '"')
            {
                filename = filename.substr(1, filename.size() - 2); // Remove the enclosing quotes
            }
            filename = StringUtils::trim(filename);
            *userdata = filename;
        }
    }
    return totalSize;
}
static size_t HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    size_t totalSize = size * nitems;
    std::string header(buffer, totalSize);

    auto splits = StringUtils::split(header, ':');
    if (splits.size() == 2)
    {
        std::map<std::string, std::string> *headers = (std::map<std::string, std::string> *)userdata;
        headers->emplace(StringUtils::trim(splits[0]), StringUtils::trim(splits[1]));
    }

    return totalSize;
}
DownloadInfo GetLinkInfo(std::string link, std::map<std::string, std::string> headers)
{
    DownloadInfo info{0, ""};
    info.size = -1;
    info.success = false;
    CURL *curl_handle = nullptr;

    curl_handle = curl_easy_init();
    struct curl_slist *headerlist = NULL;
    if (!headers.empty())
    {
        for (auto &header : headers)
        {
            std::string concatHeader = header.first + ": " + header.second;
            headerlist = curl_slist_append(headerlist, concatHeader.c_str());
        }
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0);

    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
    // curl_easy_setopt(curl_handle, CURLOPT_FORBID_REUSE, 1L);
    // curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
    if (headerlist)
    {
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerlist);
    }
    /* send all data to this function  */
    std::map<std::string, std::string> responseHeaders;
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USERAGENT.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &responseHeaders);

    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
    auto response = curl_easy_perform(curl_handle);
    auto errCount = 0;
    while (response != CURLE_OK && errCount < 5)
    {
        WaitForConnection();
        errCount++;
        response = curl_easy_perform(curl_handle);
    }
    if (headerlist)
    {
        curl_slist_free_all(headerlist);
    }
    if (response == CURLE_OK)
    {
        info.success = true;
        double size = 0;
        auto code = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
        if (code == CURLE_OK)
        {
            info.size = size > 0 ? static_cast<size_t>(size) : -1;
            info.headers = responseHeaders;
        }
        std::string fileName;
        if (responseHeaders.count("Content-Disposition"))
        {
            auto header = responseHeaders["Content-Disposition"];
            std::size_t filenamePos = header.find("filename=");
            if (filenamePos != std::string::npos)
            {
                std::string filename = header.substr(filenamePos + 9); // 9 is the length of "filename="
                if (!filename.empty() && filename[0] == '"')
                {
                    filename = filename.substr(1, filename.size() - 2); // Remove the enclosing quotes
                }
                fileName = StringUtils::trim(filename);
            }
        }

        if (fileName.empty())
        {
            auto lastSlashIdx = link.rfind("/");
            if (lastSlashIdx != std::string::npos)
            {
                fileName = link.substr(lastSlashIdx + 1);
                auto qIdx = fileName.find("?");
                if (qIdx != std::string::npos)
                {
                    fileName = fileName.substr(0, qIdx);
                }
            }
            else
            {
                fileName = "download.dat";
            }
        }
        info.name = ReplaceInvalidFileChars(fileName);
    }

    curl_easy_cleanup(curl_handle);
    return info;
}
DownloadResult DownloadFile(std::string filePath, std::string link, std::map<std::string, std::string> headers, std::atomic_bool *stopSignal, ProgressFunc progCallback)
{
    ProgressData progressArg;
    progressArg.size = 0;
    progressArg.transferred = 0;
    progressArg.eta = 0;
    size_t downloadedSize = 0;
    std::atomic<bool> completed = false;
    DownloadResult res;
    res.downloaded = 0;
    auto info = GetLinkInfo(link, headers);
    int errCount = 0;
    while (!info.success)
    {
        Sleep(2000);
        info = GetLinkInfo(link, headers);
        if (errCount > 5)
        {
            res.success = false;
            res.error = OBFUSCATED("Failed to retrive link header information");
            return res;
        }
        WaitForConnection();
        errCount++;
    }
    progressArg.name = fs::path(filePath).filename().string();
    progressArg.size = info.size;

    bool canResume = false;
    if (Exists(filePath))
    {
        if (info.size == GetFileSize(filePath))
        {
            res.downloaded = info.size;
            res.success = true;
            return res;
        }
    }
    std::error_code ec;
    auto space = fs::space(fs::path(filePath).parent_path(), ec);
    std::shared_ptr<std::thread> progThread;
    if (progCallback != nullptr)
    {
        std::thread th((std::function<void()>)[&]() {
            while (!completed)
            {
                for (size_t i = 0; i < 20; i++)
                {
                    Sleep(100);
                    if (completed) 
                    {
                        break;
                    }
                }
                progCallback(progressArg);
                res.downloaded = progressArg.transferred;
            }
        });
        progThread = std::make_shared<std::thread>(std::move(th));
    }
    if (int64_t(space.available) < info.size)
    {
        res.success = false;
        res.error = OBFUSCATED("Insufficient space available");
        return res;
    }
    auto targetPath = filePath + ".tmp";
    if (targetPath.size() > MAX_PATH)
    {
        targetPath = R"(\??\)" + targetPath;
    }
    if (Exists(targetPath))
    {
        downloadedSize = GetFileSize(targetPath);
        if (info.size == downloadedSize)
        {
            if (Exists(filePath))
            {
                try
                {
                    fs::remove(filePath);
                }
                catch (...)
                {
                }
            }
            bool moved = false;
            if (!(moved = MoveFileA(targetPath.c_str(), filePath.c_str())))
            {
                moved = CopyFileA(targetPath.c_str(), filePath.c_str(), FALSE);
            }
            res.downloaded = info.size;
            res.success = moved;
            if (!moved)
            {
                res.error = OBFUSCATED("Failed to transfer file ") + targetPath;
            }
            return res;
        }

        canResume = info.headers.count("Accept-Ranges") > 0 && info.headers["Accept-Ranges"] == "bytes";
        if (canResume)
        {
            downloadedSize = GetFileSize(targetPath);
        }
        else
        {
            downloadedSize = 0;
            if (Exists(targetPath))
            {
                try
                {
                    fs::remove(targetPath);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }
    FILE *f = fopen(targetPath.c_str(), downloadedSize > 0 ? "ab" : "wb");
    if (f == NULL)
    {
        res.success = false;
        res.error = "Failed to open file: " + targetPath;
        return res;
    }
    FileBag bag;
    bag.stopSignal = stopSignal;
    bag.file = f;
    bag.progressData = &progressArg;
    CURL *curl_handle;

    struct curl_slist *headerlist = NULL;
    /* init the curl session */
    curl_handle = curl_easy_init();

    for (auto &header : headers)
    {
        auto concatHeader = header.first + ": " + header.second;
        headerlist = curl_slist_append(headerlist, concatHeader.c_str());
    }
    /* specify URL to get */
    if (headerlist)
    {
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerlist);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0);

    /* send all data to this function  */

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, FileWriteCallback);
    if (canResume)
    {
        auto addedRange = curl_easy_setopt(curl_handle, CURLOPT_RANGE, (std::to_string(downloadedSize) + "-").c_str());
    }
    // curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&bag);

    /* some servers do not like requests that are made without a user-agent
       field, so we provide one */

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USERAGENT.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
    auto response = curl_easy_perform(curl_handle);

    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    if (headerlist)
    {
        curl_slist_free_all(headerlist);
    }
    fclose(f);

    completed = true;
    if (progThread != nullptr)
    {
        if (progThread->joinable())
        {
            progThread->join();
        }
    }
    if (progCallback != nullptr)
    {
        progressArg.transferred = bag.progressData->transferred;
        progCallback(progressArg);
        res.downloaded = progressArg.transferred;
    }
    Sleep(1000);
    if (response == CURLE_OK && (info.size < 0 || (info.size == GetFileSize(targetPath))) && (http_code >= 200 && http_code < 300))
    {
        curl_easy_cleanup(curl_handle);
        bool moved = MoveFileA(targetPath.c_str(), filePath.c_str());
        res.success = moved;
        if (!moved)
        {
            res.error = "Failed to move file";
        }
        return res;
    }
    else if (!(http_code >= 200 && http_code < 300))
    {

        res.success = false;
        res.error = "Status code error: " + std::to_string(http_code);
    }
    else if (stopSignal && *stopSignal)
    {
        res.success = false;
        res.error = "";
    }
    else
    {
        res.success = false;
        res.error = "Curl error code: " + std::to_string(response);
    }
    curl_easy_cleanup(curl_handle);
    return res;
}