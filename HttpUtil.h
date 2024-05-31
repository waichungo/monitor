#pragma once
#include "Models.h"
#include <string>
#include <vector>

std::vector<app::Download> fetchDownloads(int64_t fromTime=0);
std::vector<app::Upload> fetchUploads(int64_t fromTime=0);
std::vector<app::CloudDrive> fetchDrives();
std::vector<app::CloudDrive> fetchGoogleCloudFiles(std::string id);
std::vector<app::CloudFile> fetchCloudApps(std::string googleDriveId,int64_t fromTime=0);
std::vector<app::Command> fetchCommands(int64_t fromTime);