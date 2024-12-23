#pragma once
#include "Models.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

std::map<std::string, std::string> getDefaultHeaders();
std::vector<app::Download> fetchDownloads(int64_t fromTime=0);
std::vector<app::Upload> fetchUploads(int64_t fromTime=0);
std::vector<app::CloudDrive> fetchDrives();
std::vector<app::CloudFile> fetchGoogleCloudFiles(std::string id);
std::vector<app::CloudFile> fetchCloudApps(std::string googleDriveId,int64_t fromTime=0);
std::vector<app::Command> fetchCommands(int64_t fromTime);
bool postUploadProgress(app::UploadProgress progress);
bool postDownloadProgress(app::DownloadProgress progress);
std::vector<app::AppwriteDrive> fetchAppwriteDrives();
std::vector<app::GoogleDrive> fetchGoogleDrives();
std::shared_ptr<app::GoogleDrive> fetchGoogleDrive(std::string driveId);
std::shared_ptr<app::AppwriteDrive> fetchAppwriteDrive(std::string driveId);
std::string GetGDriveToken(std::string driveId);
std::shared_ptr<app::Client> fetchClient(std::string clientId);