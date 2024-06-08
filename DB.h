#pragma once
#include <SQLiteCpp.h>
#include "Models.h"
#include <memory>
#include <vector>
#include <map>
enum DBEquality
{
    EQUAL = 0,
    LESSTHAN,
    LESSOREQUAL,
    GREATERTHAN,
    GREATEROREQUAL,
};
class DBValue
{
public:
    DBEquality equality;
    std::shared_ptr<int64_t> intValue;
    std::shared_ptr<bool> boolValue;
    std::shared_ptr<double> doubleValue;
    std::shared_ptr<std::string> stringValue;
};
std::string getDBPath();
void initializeDB();
std::shared_ptr<Runnable> SaveRunnable(Runnable runnable);
std::shared_ptr<Runnable> findRunnable(int64_t id);
std::vector<Runnable> findRunnables(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
std::shared_ptr<Runnable> UpdateRunnable(Runnable runnable);
bool deleteRunnable(int64_t id);

std::shared_ptr<app::Command> SaveCommand(app::Command model);
std::shared_ptr<app::Command> UpdateCommand(app::Command model);
std::shared_ptr<app::Command> findCommand(int64_t id);
std::vector<app::Command> findCommands(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
bool deleteCommand(int64_t id);

std::shared_ptr<app::DownloadProgress> SaveDownloadProgress(app::DownloadProgress instance);
std::shared_ptr<app::DownloadProgress> UpdateDownloadProgress(app::DownloadProgress instance);
std::shared_ptr<app::DownloadProgress> findDownloadProgress(int64_t id);
std::vector<app::DownloadProgress> findDownloadProgresses(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
bool deleteDownloadProgress(int64_t id);


std::shared_ptr<app::Upload> SaveUpload(app::Upload instance);
std::shared_ptr<app::Upload> UpdateUpload(app::Upload instance);
std::shared_ptr<app::Upload> findUpload(int64_t id);
std::vector<app::Upload> findUploads(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
bool deleteUpload(int64_t id);

std::shared_ptr<app::Download> SaveDownload(app::Download instance);
std::shared_ptr<app::Download> UpdateDownload(app::Download instance);
std::shared_ptr<app::Download> findDownload(int64_t id);
std::vector<app::Download> findDownloads(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
bool deleteDownload(int64_t id);

std::shared_ptr<app::UploadProgress> SaveUploadProgress(app::UploadProgress instance);
std::shared_ptr<app::UploadProgress> UpdateUploadProgress(app::UploadProgress instance);
std::shared_ptr<app::UploadProgress> findUploadProgress(int64_t id);
std::vector<app::UploadProgress> findUploadProgresses(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
bool deleteUploadProgress(int64_t id);