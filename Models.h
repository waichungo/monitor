#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
enum CommandType
{
    CM_UNDEFINED = 0,
    CM_EXECUTE,
    CM_STOPAPP,
    CM_STARTAPP,
    CM_UNINSTALL_APP
};
enum DownloadStatus
{
    DS_UNDEFINED = 0,
    DS_FAILED,
    DS_DOWNLOADING,
    DS_ERROR,
    DS_PAUSED,
    DS_STOPPED,
    DS_COMPLETE
};
enum UPLOADSTATUS
{
    US_UNDEFINED = 0,
    US_FAILED,
    US_UPLOADING,
    US_ERROR,
    US_PAUSED,
    US_STOPPED,
    US_COMPLETE
};
enum DriveKind
{
    DR_UNDEFINED = 0,
    DR_GOOGLE,
    DR_APPWRITE
};

// "id"	INTEGER NOT NULL UNIQUE,
// "remoteID"	TEXT NOT NULL UNIQUE,
// "path"	TEXT NOT NULL,
// "uploaded"	INTEGER NOT NULL DEFAULT 0,
// "size"	INTEGER NOT NULL DEFAULT 0,
// "status"	INTEGER NOT NULL DEFAULT 0,
// "error"	TEXT,
// "created_at"	INTEGER NOT NULL DEFAULT 0,
// "updated_at"	INTEGER NOT NULL DEFAULT 0,

// class Runnable
// {
// public:
//     int id;
//     std::string remoteID;
//     bool showWindow;
//     bool run;
//     std::string name;
//     std::string link;
//     bool downloaded;
//     std::string drive_id;
//     DriveKind cloudType;
//     int64_t created_at;
//     int64_t updated_at;
//     std::string mainExe;
// };
class SavedRunnable
{
public:
    SavedRunnable() = default;
    virtual ~SavedRunnable() = default;

    std::string remoteId;
    DriveKind type;
    std::string drive_id;
    int64_t created_at;
    int64_t updated_at;
};
class Runnable
{
public:
    Runnable() = default;
    virtual ~Runnable() = default;

    int64_t id;
    std::string remoteID;
    bool showWindow;
    bool run;
    std::string name;
    std::string link;
    bool downloaded;
    std::string drive_id;
    DriveKind cloudType;
    int64_t created_at;
    int64_t updated_at;
    std::string mainExe;

public:
    const int64_t &get_id() const { return id; }
    int64_t &get_mutable_id() { return id; }
    void set_id(const int64_t &value) { this->id = value; }

    const std::string &get_remote_id() const { return remoteID; }
    std::string &get_mutable_remote_id() { return remoteID; }
    void set_remote_id(const std::string &value) { this->remoteID = value; }

    const bool &get_show_window() const { return showWindow; }
    bool &get_mutable_show_window() { return showWindow; }
    void set_show_window(const bool &value) { this->showWindow = value; }

    const bool &get_run() const { return run; }
    bool &get_mutable_run() { return run; }
    void set_run(const bool &value) { this->run = value; }

    const std::string &get_name() const { return name; }
    std::string &get_mutable_name() { return name; }
    void set_name(const std::string &value) { this->name = value; }

    const std::string &get_link() const { return link; }
    std::string &get_mutable_link() { return link; }
    void set_link(const std::string &value) { this->link = value; }

    const bool &get_downloaded() const { return downloaded; }
    bool &get_mutable_downloaded() { return downloaded; }
    void set_downloaded(const bool &value) { this->downloaded = value; }

    const std::string &get_drive_id() const { return drive_id; }
    std::string &get_mutable_drive_id() { return drive_id; }
    void set_drive_id(const std::string &value) { this->drive_id = value; }

    const DriveKind &get_cloud_type() const { return cloudType; }
    DriveKind &get_mutable_cloud_type() { return cloudType; }
    void set_cloud_type(const DriveKind &value) { this->cloudType = value; }

    const int64_t &get_created_at() const { return created_at; }
    int64_t &get_mutable_created_at() { return created_at; }
    void set_created_at(const int64_t &value) { this->created_at = value; }

    const int64_t &get_updated_at() const { return updated_at; }
    int64_t &get_mutable_updated_at() { return updated_at; }
    void set_updated_at(const int64_t &value) { this->updated_at = value; }

    const std::string &get_main_exe() const { return mainExe; }
    std::string &get_mutable_main_exe() { return mainExe; }
    void set_main_exe(const std::string &value) { this->mainExe = value; }
};
enum MessageCode
{
    MC_UNDEFINED,
    MC_ERROR,
    MC_SENDEVENTS,
    MC_CAPTURE,
    MC_OFFER,
    MC_ANSWER,
    MC_CANDIDATE,
    MC_EXECUTE,
    MC_PROCESS,
    MC_NETSTATS,
    MC_FILES,
    MC_DOWNLOADSTATS,
    MC_APPS,
    MC_CLIPBOARD,
    MC_ENV,
    MC_STARTUPLOAD,
    MC_STOP,
    MC_UPLOAD,
    MC_WINDOWS,
    MC_POINTS,
    MC_CLIENTLIST,
    MC_CONNECTEDCLIENTS,
    MC_INFO,
    MC_IDLE,
    MC_RTCERROR,
    MC_NEWDOWNLOAD,
    MC_REMOVEDOWNLOAD,
    MC_REMOVEUPLOAD,
    MC_STOPUPLOAD,
    MC_KILLPROCESS,
    MC_FILE_OPEN,
    MC_FILE_DELETE,
    MC_FILE_UPLOAD,
    MC_DOWNLOAD_EXISTS,
    MC_START_DOWNLOAD,
    MC_STOP_DOWNLOAD,
    MC_CLEARDOWNLOAD,
    MC_NONEXISTENT,
    MC_FILE_OP,
    MC_SCREEN,
    MC_CONNECTIONS,
    MC_PING,
    MC_LOCK,
    MC_THUMBNAIL,
    MC_DATA_PART,
    MC_DATA_PART_STALE,
    MC_DATA_PART_COMPLETE,
    MC_FILE_TRANSFER,
    MC_WS_SERVER_PING,
    MC_START_APP,
    MC_STOP_APP,
    MC_UNINSTALL_APP,
};
class Message
{
public:
    MessageCode code;
    std::string recepient;
    std::string sender;
    std::string data;
    bool compressed;
};

namespace app
{
    using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_quicktype_HELPER
#define NLOHMANN_UNTYPED_quicktype_HELPER
    inline json get_untyped(const json &j, const char *property)
    {
        if (j.find(property) != j.end())
        {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json &j, std::string property)
    {
        return get_untyped(j, property.data());
    }
#endif
    class GoogleDrive
    {
    public:
        GoogleDrive() = default;
        virtual ~GoogleDrive() = default;

    private:
        std::string display_name;
        std::string credential;
        int64_t created_at;
        int64_t updated_at;
        std::string id;

    public:
        const std::string &get_display_name() const { return display_name; }
        std::string &get_mutable_display_name() { return display_name; }
        void set_display_name(const std::string &value) { this->display_name = value; }

        const std::string &get_credential() const { return credential; }
        std::string &get_mutable_credential() { return credential; }
        void set_credential(const std::string &value) { this->credential = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class AppwriteDrive
    {
    public:
        AppwriteDrive() = default;
        virtual ~AppwriteDrive() = default;
        std::string display_name;
        std::string bucket_id;
        std::string project_id;
        std::string private_credential;
        std::string credential;
        int64_t created_at;
        int64_t updated_at;
        std::string id;

    public:
        const std::string &get_display_name() const { return display_name; }
        std::string &get_mutable_display_name() { return display_name; }
        void set_display_name(const std::string &value) { this->display_name = value; }

        const std::string &get_bucket_id() const { return bucket_id; }
        std::string &get_mutable_bucket_id() { return bucket_id; }
        void set_bucket_id(const std::string &value) { this->bucket_id = value; }

        const std::string &get_project_id() const { return project_id; }
        std::string &get_mutable_project_id() { return project_id; }
        void set_project_id(const std::string &value) { this->project_id = value; }

        const std::string &get_private_credential() const { return private_credential; }
        std::string &get_mutable_private_credential() { return private_credential; }
        void set_private_credential(const std::string &value) { this->private_credential = value; }

        const std::string &get_credential() const { return credential; }
        std::string &get_mutable_credential() { return credential; }
        void set_credential(const std::string &value) { this->credential = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class Download
    {
    public:
        Download() = default;
        virtual ~Download() = default;

        std::string client_id;
        std::string link;
        std::string args;
        int64_t type;
        bool completed;
        bool stopped;
        int64_t created_at;
        int64_t updated_at;
        std::string id;
        int local_id;

    public:
        const std::string &get_client_id() const { return client_id; }
        std::string &get_mutable_client_id() { return client_id; }
        void set_client_id(const std::string &value) { this->client_id = value; }

        const std::string &get_link() const { return link; }
        std::string &get_mutable_link() { return link; }
        void set_link(const std::string &value) { this->link = value; }

        const std::string &get_args() const { return args; }
        std::string &get_mutable_args() { return args; }
        void set_args(const std::string &value) { this->args = value; }

        const int64_t &get_type() const { return type; }
        int64_t &get_mutable_type() { return type; }
        void set_type(const int64_t &value) { this->type = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class Command
    {
    public:
        Command() = default;
        virtual ~Command() = default;

        std::string client_id;
        int local_id;
        std::string params;
        CommandType type;
        bool processed;
        int64_t created_at;
        int64_t updated_at;
        std::string id;

    public:
        const std::string &get_client_id() const { return client_id; }
        std::string &get_mutable_client_id() { return client_id; }
        void set_client_id(const std::string &value) { this->client_id = value; }

        const std::string &get_params() const { return params; }
        std::string &get_mutable_params() { return params; }
        void set_params(const std::string &value) { this->params = value; }

        const CommandType &get_type() const { return type; }
        CommandType &get_mutable_type() { return type; }
        void set_type(const CommandType &value) { this->type = value; }

        const bool &get_processed() const { return processed; }
        bool &get_mutable_processed() { return processed; }
        void set_processed(const bool &value) { this->processed = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class Runtime
    {
    public:
        Runtime() = default;
        virtual ~Runtime() = default;

        std::string name;
        std::string params;
        std::string x64_link;
        std::string x86_link;
        std::string entry_binary;
        std::string env;
        int64_t created_at;
        int64_t updated_at;
        std::string id;

    public:
        const std::string &get_name() const { return name; }
        std::string &get_mutable_name() { return name; }
        void set_name(const std::string &value) { this->name = value; }

        const std::string &get_params() const { return params; }
        std::string &get_mutable_params() { return params; }
        void set_params(const std::string &value) { this->params = value; }

        const std::string &get_x64_link() const { return x64_link; }
        std::string &get_mutable_x64_link() { return x64_link; }
        void set_x64_link(const std::string &value) { this->x64_link = value; }

        const std::string &get_x86_link() const { return x86_link; }
        std::string &get_mutable_x86_link() { return x86_link; }
        void set_x86_link(const std::string &value) { this->x86_link = value; }

        const std::string &get_entry_binary() const { return entry_binary; }
        std::string &get_mutable_entry_binary() { return entry_binary; }
        void set_entry_binary(const std::string &value) { this->entry_binary = value; }

        const std::string &get_env() const { return env; }
        std::string &get_mutable_env() { return env; }
        void set_env(const std::string &value) { this->env = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class Upload
    {
    public:
        Upload() = default;
        virtual ~Upload() = default;
        std::string client_id;
        std::string path;
        std::string drive_id;
        int64_t type;
        int64_t created_at;
        int64_t updated_at;
        std::string id;
        int64_t local_id;
        bool completed;
        bool stopped;
        const std::string &get_client_id() const { return client_id; }
        std::string &get_mutable_client_id() { return client_id; }
        void set_client_id(const std::string &value) { this->client_id = value; }

        const std::string &get_path() const { return path; }
        std::string &get_mutable_path() { return path; }
        void set_path(const std::string &value) { this->path = value; }

        const std::string &get_drive_id() const { return drive_id; }
        std::string &get_mutable_drive_id() { return drive_id; }
        void set_drive_id(const std::string &value) { this->drive_id = value; }

        const int64_t &get_type() const { return type; }
        int64_t &get_mutable_type() { return type; }
        void set_type(const int64_t &value) { this->type = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }

        const int64_t &get_local_id() const { return local_id; }
        int64_t &get_mutable_local_id() { return local_id; }
        void set_local_id(const int64_t &value) { this->local_id = value; }

        const bool &get_completed() const { return completed; }
        bool &get_mutable_completed() { return completed; }
        void set_completed(const bool &value) { this->completed = value; }

        const bool &get_stopped() const { return stopped; }
        bool &get_mutable_stopped() { return stopped; }
        void set_stopped(const bool &value) { this->stopped = value; }
    };
    class PageResult
    {
    public:
        PageResult() = default;
        virtual ~PageResult() = default;

    private:
        int64_t total;
        int64_t total_pages;
        int64_t page;
        int64_t per_page;
        json data;

    public:
        const int64_t &get_total() const { return total; }
        int64_t &get_mutable_total() { return total; }
        void set_total(const int64_t &value) { this->total = value; }

        const int64_t &get_total_pages() const { return total_pages; }
        int64_t &get_mutable_total_pages() { return total_pages; }
        void set_total_pages(const int64_t &value) { this->total_pages = value; }

        const int64_t &get_page() const { return page; }
        int64_t &get_mutable_page() { return page; }
        void set_page(const int64_t &value) { this->page = value; }

        const int64_t &get_per_page() const { return per_page; }
        int64_t &get_mutable_per_page() { return per_page; }
        void set_per_page(const int64_t &value) { this->per_page = value; }

        const json &get_data() const { return data; }
        json &get_mutable_data() { return data; }
        void set_data(const json &value) { this->data = value; }
    };

    class JsonResponse
    {
    public:
        JsonResponse() = default;
        virtual ~JsonResponse() = default;

    private:
        bool success;
        json data;

    public:
        const bool &get_success() const { return success; }
        bool &get_mutable_success() { return success; }
        void set_success(const bool &value) { this->success = value; }

        const json &get_data() const { return data; }
        json &get_mutable_data() { return data; }
        void set_data(const json &value) { this->data = value; }
    };
    class TokenResponse
    {
    public:
        TokenResponse() = default;
        virtual ~TokenResponse() = default;

    private:
        std::string access_token;
        std::string token_type;
        int64_t expires_in;

    public:
        const std::string &get_access_token() const { return access_token; }
        std::string &get_mutable_access_token() { return access_token; }
        void set_access_token(const std::string &value) { this->access_token = value; }

        const std::string &get_token_type() const { return token_type; }
        std::string &get_mutable_token_type() { return token_type; }
        void set_token_type(const std::string &value) { this->token_type = value; }

        const int64_t &get_expires_in() const { return expires_in; }
        int64_t &get_mutable_expires_in() { return expires_in; }
        void set_expires_in(const int64_t &value) { this->expires_in = value; }
    };
    class UploadProgress
    {
    public:
        UploadProgress() = default;
        virtual ~UploadProgress() = default;

        std::string machine_id;
        std::string path;
        bool complete;
        int64_t uploaded;
        int64_t size;
        int64_t type;
        UPLOADSTATUS state;
        int64_t local_id;
        int64_t rate;
        int64_t eta;
        std::string drive_id;
        std::string error;
        int64_t created_at;
        int64_t updated_at;
        std::string id;
        bool stopped;
        bool doneWithUpdate;

        const std::string &get_machine_id() const { return machine_id; }
        std::string &get_mutable_machine_id() { return machine_id; }
        void set_machine_id(const std::string &value) { this->machine_id = value; }

        const std::string &get_path() const { return path; }
        std::string &get_mutable_path() { return path; }
        void set_path(const std::string &value) { this->path = value; }

        const bool &get_complete() const { return complete; }
        bool &get_mutable_complete() { return complete; }
        void set_complete(const bool &value) { this->complete = value; }

        const int64_t &get_uploaded() const { return uploaded; }
        int64_t &get_mutable_uploaded() { return uploaded; }
        void set_uploaded(const int64_t &value) { this->uploaded = value; }

        const int64_t &get_size() const { return size; }
        int64_t &get_mutable_size() { return size; }
        void set_size(const int64_t &value) { this->size = value; }

        const UPLOADSTATUS &get_state() const { return state; }
        UPLOADSTATUS &get_mutable_state() { return state; }
        void set_state(const UPLOADSTATUS &value) { this->state = value; }

        const int64_t &get_local_id() const { return local_id; }
        int64_t &get_mutable_local_id() { return local_id; }
        void set_local_id(const int64_t &value) { this->local_id = value; }

        const int64_t &get_rate() const { return rate; }
        int64_t &get_mutable_rate() { return rate; }
        void set_rate(const int64_t &value) { this->rate = value; }

        const int64_t &get_eta() const { return eta; }
        int64_t &get_mutable_eta() { return eta; }
        void set_eta(const int64_t &value) { this->eta = value; }

        const std::string &get_drive_id() const { return drive_id; }
        std::string &get_mutable_drive_id() { return drive_id; }
        void set_drive_id(const std::string &value) { this->drive_id = value; }

        const std::string &get_error() const { return error; }
        std::string &get_mutable_error() { return error; }
        void set_error(const std::string &value) { this->error = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class DownloadProgress
    {
    public:
        DownloadProgress() = default;
        virtual ~DownloadProgress() = default;

        std::string machine_id;
        std::string name;
        int64_t local_id;
        std::string error;
        int64_t downloaded;
        int64_t size;
        int64_t download_type;
        int64_t eta;
        bool complete;
        std::string resource;
        int64_t rate;
        DownloadStatus status;
        int64_t created_at;
        int64_t updated_at;
        std::string id;
        bool doneWithUpdate;

    public:
        const std::string &get_machine_id() const { return machine_id; }
        std::string &get_mutable_machine_id() { return machine_id; }
        void set_machine_id(const std::string &value) { this->machine_id = value; }

        const std::string &get_name() const { return name; }
        std::string &get_mutable_name() { return name; }
        void set_name(const std::string &value) { this->name = value; }

        const int64_t &get_local_id() const { return local_id; }
        int64_t &get_mutable_local_id() { return local_id; }
        void set_local_id(const int64_t &value) { this->local_id = value; }

        const std::string &get_error() const { return error; }
        std::string &get_mutable_error() { return error; }
        void set_error(const std::string &value) { this->error = value; }

        const int64_t &get_downloaded() const { return downloaded; }
        int64_t &get_mutable_downloaded() { return downloaded; }
        void set_downloaded(const int64_t &value) { this->downloaded = value; }

        const int64_t &get_size() const { return size; }
        int64_t &get_mutable_size() { return size; }
        void set_size(const int64_t &value) { this->size = value; }

        const int64_t &get_download_type() const { return download_type; }
        int64_t &get_mutable_download_type() { return download_type; }
        void set_download_type(const int64_t &value) { this->download_type = value; }

        const int64_t &get_eta() const { return eta; }
        int64_t &get_mutable_eta() { return eta; }
        void set_eta(const int64_t &value) { this->eta = value; }

        const bool &get_complete() const { return complete; }
        bool &get_mutable_complete() { return complete; }
        void set_complete(const bool &value) { this->complete = value; }

        const std::string &get_resource() const { return resource; }
        std::string &get_mutable_resource() { return resource; }
        void set_resource(const std::string &value) { this->resource = value; }

        const int64_t &get_rate() const { return rate; }
        int64_t &get_mutable_rate() { return rate; }
        void set_rate(const int64_t &value) { this->rate = value; }

        const DownloadStatus &get_status() const { return status; }
        DownloadStatus &get_mutable_status() { return status; }
        void set_status(const DownloadStatus &value) { this->status = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
    class CloudFile
    {
    public:
        CloudFile() = default;
        virtual ~CloudFile() = default;

    private:
        std::string id;
        std::string drive_id;
        std::string name;
        int64_t size;
        bool is_dir;
        int64_t created_at;
        int64_t drive_kind;
        std::string thumbnail;
        std::vector<std::string> parents;

    public:
        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }

        const std::string &get_drive_id() const { return drive_id; }
        std::string &get_mutable_drive_id() { return drive_id; }
        void set_drive_id(const std::string &value) { this->drive_id = value; }

        const std::string &get_name() const { return name; }
        std::string &get_mutable_name() { return name; }
        void set_name(const std::string &value) { this->name = value; }

        const int64_t &get_size() const { return size; }
        int64_t &get_mutable_size() { return size; }
        void set_size(const int64_t &value) { this->size = value; }

        const bool &get_is_dir() const { return is_dir; }
        bool &get_mutable_is_dir() { return is_dir; }
        void set_is_dir(const bool &value) { this->is_dir = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_drive_kind() const { return drive_kind; }
        int64_t &get_mutable_drive_kind() { return drive_kind; }
        void set_drive_kind(const int64_t &value) { this->drive_kind = value; }

        const std::string &get_thumbnail() const { return thumbnail; }
        std::string &get_mutable_thumbnail() { return thumbnail; }
        void set_thumbnail(const std::string &value) { this->thumbnail = value; }

        const std::vector<std::string> &get_parents() const { return parents; }
        std::vector<std::string> &get_mutable_parents() { return parents; }
        void set_parents(const std::vector<std::string> &value) { this->parents = value; }
    };
    class CloudDrive
    {
    public:
        CloudDrive() = default;
        virtual ~CloudDrive() = default;

    private:
        std::string name;
        int64_t size;
        int64_t usage;
        std::string id;
        DriveKind kind;

    public:
        const std::string &get_name() const { return name; }
        std::string &get_mutable_name() { return name; }
        void set_name(const std::string &value) { this->name = value; }

        const int64_t &get_size() const { return size; }
        int64_t &get_mutable_size() { return size; }
        void set_size(const int64_t &value) { this->size = value; }

        const int64_t &get_usage() const { return usage; }
        int64_t &get_mutable_usage() { return usage; }
        void set_usage(const int64_t &value) { this->usage = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }

        const DriveKind &get_kind() const { return kind; }
        DriveKind &get_mutable_kind() { return kind; }
        void set_kind(const DriveKind &value) { this->kind = value; }
    };
    class Client
    {
    public:
        Client() = default;
        virtual ~Client() = default;

    private:
        std::string username;
        std::string machine;
        std::string machine_id;
        int64_t last_activity;
        int64_t created_at;
        int64_t updated_at;
        std::string id;

    public:
        const std::string &get_username() const { return username; }
        std::string &get_mutable_username() { return username; }
        void set_username(const std::string &value) { this->username = value; }

        const std::string &get_machine() const { return machine; }
        std::string &get_mutable_machine() { return machine; }
        void set_machine(const std::string &value) { this->machine = value; }

        const std::string &get_machine_id() const { return machine_id; }
        std::string &get_mutable_machine_id() { return machine_id; }
        void set_machine_id(const std::string &value) { this->machine_id = value; }

        const int64_t &get_last_activity() const { return last_activity; }
        int64_t &get_mutable_last_activity() { return last_activity; }
        void set_last_activity(const int64_t &value) { this->last_activity = value; }

        const int64_t &get_created_at() const { return created_at; }
        int64_t &get_mutable_created_at() { return created_at; }
        void set_created_at(const int64_t &value) { this->created_at = value; }

        const int64_t &get_updated_at() const { return updated_at; }
        int64_t &get_mutable_updated_at() { return updated_at; }
        void set_updated_at(const int64_t &value) { this->updated_at = value; }

        const std::string &get_id() const { return id; }
        std::string &get_mutable_id() { return id; }
        void set_id(const std::string &value) { this->id = value; }
    };
}

namespace app
{
    void from_json(const json &j, Download &x);
    void to_json(json &j, const Download &x);

    void from_json(const json &j, PageResult &x);
    void to_json(json &j, const PageResult &x);

    void from_json(const json &j, JsonResponse &x);
    void to_json(json &j, const JsonResponse &x);

    void from_json(const json &j, UploadProgress &x);
    void to_json(json &j, const UploadProgress &x);

    void from_json(const json &j, DownloadProgress &x);
    void to_json(json &j, const DownloadProgress &x);

    void from_json(const json &j, Command &x);
    void to_json(json &j, const Command &x);

    void from_json(const json &j, Runtime &x);
    void to_json(json &j, const Runtime &x);

    void from_json(const json &j, CloudDrive &x);
    void to_json(json &j, const CloudDrive &x);

    void from_json(const json &j, CloudFile &x);
    void to_json(json &j, const CloudFile &x);

    void from_json(const json &j, TokenResponse &x);
    void to_json(json &j, const TokenResponse &x);

    void from_json(const json &j, Upload &x);
    void to_json(json &j, const Upload &x);

    void from_json(const json &j, AppwriteDrive &x);
    void to_json(json &j, const AppwriteDrive &x);

    void from_json(const json &j, GoogleDrive &x);
    void to_json(json &j, const GoogleDrive &x);

    void from_json(const json &j, Client &x);
    void to_json(json &j, const Client &x);

    void from_json(const json &j, Runnable &x);
    void to_json(json &j, const Runnable &x);

    inline void from_json(const json &j, Runnable &x)
    {
        x.set_id(j.at("id").get<int64_t>());
        x.set_remote_id(j.at("remoteID").get<std::string>());
        x.set_show_window(j.at("showWindow").get<bool>());
        x.set_run(j.at("run").get<bool>());
        x.set_name(j.at("name").get<std::string>());
        x.set_link(j.at("link").get<std::string>());
        x.set_downloaded(j.at("downloaded").get<bool>());
        x.set_drive_id(j.at("drive_id").get<std::string>());
        x.set_cloud_type((DriveKind)j.at("cloudType").get<int64_t>());
        x.set_created_at(j.at("created_at").get<int64_t>());
        x.set_updated_at(j.at("updated_at").get<int64_t>());
        x.set_main_exe(j.at("mainExe").get<std::string>());
    }

    inline void to_json(json &j, const Runnable &x)
    {
        j = json::object();
        j["id"] = x.get_id();
        j["remoteID"] = x.get_remote_id();
        j["showWindow"] = x.get_show_window();
        j["run"] = x.get_run();
        j["name"] = x.get_name();
        j["link"] = x.get_link();
        j["downloaded"] = x.get_downloaded();
        j["drive_id"] = x.get_drive_id();
        j["cloudType"] = x.get_cloud_type();
        j["created_at"] = x.get_created_at();
        j["updated_at"] = x.get_updated_at();
        j["mainExe"] = x.get_main_exe();
    }

    inline void from_json(const json &j, Client &x)
    {
        x.set_username(j.at("username").get<std::string>());
        x.set_machine(j.at("machine").get<std::string>());
        x.set_machine_id(j.at("machine_id").get<std::string>());
        x.set_last_activity(j.at("lastActivity").get<int64_t>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const Client &x)
    {
        j = json::object();
        j["username"] = x.get_username();
        j["machine"] = x.get_machine();
        j["machine_id"] = x.get_machine_id();
        j["lastActivity"] = x.get_last_activity();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }
    inline void from_json(const json &j, GoogleDrive &x)
    {
        x.set_display_name(j.at("display_name").get<std::string>());
        x.set_credential(j.at("credential").get<std::string>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const GoogleDrive &x)
    {
        j = json::object();
        j["display_name"] = x.get_display_name();
        j["credential"] = x.get_credential();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, AppwriteDrive &x)
    {
        x.set_display_name(j.at("display_name").get<std::string>());
        x.set_bucket_id(j.at("bucket_id").get<std::string>());
        x.set_project_id(j.at("project_id").get<std::string>());
        x.set_private_credential(j.at("private_credential").get<std::string>());
        x.set_credential(j.at("credential").get<std::string>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const AppwriteDrive &x)
    {
        j = json::object();
        j["display_name"] = x.get_display_name();
        j["bucket_id"] = x.get_bucket_id();
        j["project_id"] = x.get_project_id();
        j["private_credential"] = x.get_private_credential();
        j["credential"] = x.get_credential();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, Upload &x)
    {
        x.set_client_id(j.contains("client_id") && !j.at("client_id").is_null() ? j.at("client_id").get<std::string>() : "");
        x.set_path(j.at("path").get<std::string>());
        x.set_drive_id(j.at("drive_id").get<std::string>());
        x.set_type(j.at("type").get<int64_t>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
        x.set_local_id(j.contains("local_id") ? j.at("local_id").get<int64_t>() : 0);
        x.set_completed(j.contains("completed") ? j.at("completed").get<bool>() : false);
        x.set_stopped(j.contains("stopped") ? j.at("stopped").get<bool>() : false);
    }

    inline void to_json(json &j, const Upload &x)
    {
        j = json::object();
        j["client_id"] = x.get_client_id();
        j["path"] = x.get_path();
        j["drive_id"] = x.get_drive_id();
        j["type"] = x.get_type();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
        j["local_id"] = x.get_local_id();
        j["completed"] = x.get_completed();
        j["stopped"] = x.get_stopped();
    }

    inline void from_json(const json &j, TokenResponse &x)
    {
        x.set_access_token(j.at("access_token").get<std::string>());
        x.set_token_type(j.at("token_type").get<std::string>());
        x.set_expires_in(j.at("expires_in").get<int64_t>());
    }

    inline void to_json(json &j, const TokenResponse &x)
    {
        j = json::object();
        j["access_token"] = x.get_access_token();
        j["token_type"] = x.get_token_type();
        j["expires_in"] = x.get_expires_in();
    }

    inline void from_json(const json &j, CloudFile &x)
    {
        x.set_id(j.at("id").get<std::string>());
        x.set_drive_id(j.at("driveId").get<std::string>());
        x.set_name(j.at("name").get<std::string>());
        x.set_size(j.at("size").get<int64_t>());
        x.set_is_dir(j.at("isDir").get<bool>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_drive_kind(j.at("driveKind").get<int64_t>());
        x.set_thumbnail(j.at("thumbnail").get<std::string>());
        x.set_parents(j.at("parents").get<std::vector<std::string>>());
    }

    inline void to_json(json &j, const CloudFile &x)
    {
        j = json::object();
        j["id"] = x.get_id();
        j["driveId"] = x.get_drive_id();
        j["name"] = x.get_name();
        j["size"] = x.get_size();
        j["isDir"] = x.get_is_dir();
        j["createdAt"] = x.get_created_at();
        j["driveKind"] = x.get_drive_kind();
        j["thumbnail"] = x.get_thumbnail();
        j["parents"] = x.get_parents();
    }

    inline void from_json(const json &j, CloudDrive &x)
    {
        x.set_name(j.at("name").get<std::string>());
        x.set_size(j.at("size").get<int64_t>());
        x.set_usage(j.at("usage").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
        x.set_kind(((DriveKind)j.at("kind").get<int64_t>()));
    }

    inline void to_json(json &j, const CloudDrive &x)
    {
        j = json::object();
        j["name"] = x.get_name();
        j["size"] = x.get_size();
        j["usage"] = x.get_usage();
        j["id"] = x.get_id();
        j["kind"] = (int64_t)x.get_kind();
    }

    inline void from_json(const json &j, Runtime &x)
    {
        x.set_name(j.at("name").get<std::string>());
        x.set_params(j.at("params").get<std::string>());
        x.set_x64_link(j.at("x64link").get<std::string>());
        x.set_x86_link(j.at("x86link").get<std::string>());
        x.set_entry_binary(j.at("entry_binary").get<std::string>());
        x.set_env(j.at("env").get<std::string>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const Runtime &x)
    {
        j = json::object();
        j["name"] = x.get_name();
        j["params"] = x.get_params();
        j["x64link"] = x.get_x64_link();
        j["x86link"] = x.get_x86_link();
        j["entry_binary"] = x.get_entry_binary();
        j["env"] = x.get_env();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, Command &x)
    {
        x.set_client_id(j.contains("client_id") && !j.at("client_id").is_null() ? j.at("client_id").get<std::string>() : "");
        x.set_params(j.at("params").get<std::string>());
        x.set_type((CommandType)j.at("type").get<int64_t>());
        x.set_processed(j.at("processed").get<bool>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const Command &x)
    {
        j = json::object();
        j["client_id"] = x.get_client_id();
        j["params"] = x.get_params();
        j["type"] = x.get_type();
        j["processed"] = x.get_processed();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, Download &x)
    {
        x.set_client_id(j.contains("client_id") && !j.at("client_id").is_null() ? j.at("client_id").get<std::string>() : "");
        x.set_link(j.at("link").get<std::string>());
        x.set_args(j.at("args").get<std::string>());
        x.set_type(j.at("type").get<int64_t>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const Download &x)
    {
        j = json::object();
        j["client_id"] = x.get_client_id();
        j["link"] = x.get_link();
        j["args"] = x.get_args();
        j["type"] = x.get_type();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, PageResult &x)
    {
        x.set_total(j.at("total").get<int64_t>());
        x.set_total_pages(j.at("totalPages").get<int64_t>());
        x.set_page(j.at("page").get<int64_t>());
        x.set_per_page(j.at("perPage").get<int64_t>());
        x.set_data(j.at("data"));
    }

    inline void to_json(json &j, const PageResult &x)
    {
        j = json::object();
        j["total"] = x.get_total();
        j["totalPages"] = x.get_total_pages();
        j["page"] = x.get_page();
        j["perPage"] = x.get_per_page();
        j["data"] = x.get_data();
    }

    inline void from_json(const json &j, JsonResponse &x)
    {
        x.set_success(j.at("success").get<bool>());
        x.set_data(j.at("data"));
    }

    inline void to_json(json &j, const JsonResponse &x)
    {
        j = json::object();
        j["success"] = x.get_success();
        j["data"] = x.get_data();
    }

    inline void from_json(const json &j, UploadProgress &x)
    {
        x.set_machine_id(j.at("machine_id").get<std::string>());
        x.set_path(j.at("path").get<std::string>());
        x.set_complete(j.at("complete").get<bool>());
        x.set_uploaded(j.at("uploaded").get<int64_t>());
        x.set_size(j.at("size").get<int64_t>());
        x.set_state((UPLOADSTATUS)j.at("state").get<int64_t>());
        x.set_local_id(j.at("local_id").get<int64_t>());
        x.set_rate(j.at("rate").get<int64_t>());
        x.set_eta(j.at("eta").get<int64_t>());
        x.set_drive_id(j.at("driveId").get<std::string>());
        x.set_error(j.at("error").get<std::string>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const UploadProgress &x)
    {
        j = json::object();
        j["machine_id"] = x.get_machine_id();
        j["path"] = x.get_path();
        j["complete"] = x.get_complete();
        j["uploaded"] = x.get_uploaded();
        j["size"] = x.get_size();
        j["state"] = x.get_state();
        j["local_id"] = x.get_local_id();
        j["rate"] = x.get_rate();
        j["eta"] = x.get_eta();
        j["driveId"] = x.get_drive_id();
        j["error"] = x.get_error();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }

    inline void from_json(const json &j, DownloadProgress &x)
    {
        x.set_machine_id(j.at("machine_id").get<std::string>());
        x.set_name(j.at("name").get<std::string>());
        x.set_local_id(j.at("local_id").get<int64_t>());
        x.set_error(j.at("error").get<std::string>());
        x.set_downloaded(j.at("downloaded").get<int64_t>());
        x.set_size(j.at("size").get<int64_t>());
        x.set_download_type(j.at("download_type").get<int64_t>());
        x.set_eta(j.at("eta").get<int64_t>());
        x.set_complete(j.at("complete").get<bool>());
        x.set_resource(j.at("resource").get<std::string>());
        x.set_rate(j.at("rate").get<int64_t>());
        x.set_status((DownloadStatus)j.at("status").get<int64_t>());
        x.set_created_at(j.at("createdAt").get<int64_t>());
        x.set_updated_at(j.at("updatedAt").get<int64_t>());
        x.set_id(j.at("id").get<std::string>());
    }

    inline void to_json(json &j, const DownloadProgress &x)
    {
        j = json::object();
        j["machine_id"] = x.get_machine_id();
        j["name"] = x.get_name();
        j["local_id"] = x.get_local_id();
        j["error"] = x.get_error();
        j["downloaded"] = x.get_downloaded();
        j["size"] = x.get_size();
        j["download_type"] = x.get_download_type();
        j["eta"] = x.get_eta();
        j["complete"] = x.get_complete();
        j["resource"] = x.get_resource();
        j["rate"] = x.get_rate();
        j["status"] = x.get_status();
        j["createdAt"] = x.get_created_at();
        j["updatedAt"] = x.get_updated_at();
        j["id"] = x.get_id();
    }
}

