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
UploadResponse uploadGoogleDriveFile(std::string file, app::GoogleDrive drive, std::atomic_bool  *stopSignal=nullptr,ProgressFunc progressCallback=nullptr );
UploadResponse uploadFile(std::string file, std::string uploadLink, std::map<std::string, std::string> bodyFields={}, std::map<std::string, std::string> headers={}, std::atomic_bool *stopSignal=nullptr,ProgressFunc progressCallback=nullptr );
void uploadUpload(app::Upload upload);