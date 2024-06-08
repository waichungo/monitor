#pragma once
#include <string>
#include <vector>
#include "HttpUtil.h"
#include "Downloader.h"

typedef struct UploadResponse
{
    std::string response;
    std::string error;
    int code;
    bool success;

} UploadResponse;

UploadResponse uploadAppwriteFile(std::string file, app::AppwriteDrive drive, std::atomic_bool *stopSignal=nullptr,ProgressFunc progressCallback=nullptr );
UploadResponse uploadFile(std::string file, std::string uploadLink, std::map<string, string> bodyFields={}, std::map<string, string> headers={}, std::atomic_bool *stopSignal=nullptr,ProgressFunc progressCallback=nullptr );