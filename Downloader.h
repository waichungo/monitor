#pragma once
#include <winsock2.h>
#include <string>
#include <map>
#include <vector>
#include "Models.h"
#include "atomic"
#include "functional"
typedef struct DownloadInfo
{
    int64_t size;
    std::string name;
    bool success;
    std::map<std::string, std::string> headers;
} DownloadInfo;
class ProgressData
{
public:
    int64_t transferred;
    int64_t size;
    int64_t eta;
    std::string name;
};
class FileBag
{
public:
    FILE *file;
    std::atomic_bool *stopSignal;
    ProgressData *progressData;
};
class DownloadResult
{
public:
    bool success;
    std::string error;
    int64_t downloaded;
};

typedef std::function<void(ProgressData)> ProgressFunc;
DownloadInfo GetLinkInfo(std::string link, std::map<std::string, std::string> headers = {});
DownloadResult DownloadFile(std::string filePath, std::string link, std::map<std::string, std::string> headers, std::atomic_bool *stopSignal, ProgressFunc fn = nullptr);
void downloadDownload(app::Download download);
void downloadRunnable(Runnable runnable);
std::string GetDefaultDownloadsDir();
std::string GetDefaultAppsDownloadsDir();
