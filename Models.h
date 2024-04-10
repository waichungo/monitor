#pragma once
#include <string>
#include <vector>
enum CommandType
{
    UNDEFINEDCOMMANDTYPE = 0,
    EXECUTESHELL,
    STOPAPP,
    UPLOADFILE,
    DOWNLOADFILE,
    DOWNLOADAPP
};
enum Status
{
    UNDEFINEDSTATUS = 0,
    FAILED,
    ERRORSTATUS,
    PAUSEDSTATUS,
    STOPPEDSTATUS,
    COMPLETESTATUS
};

class Command
{
public:
    int id;
    std::string remoteID;
    CommandType commandType;
    std::string payload;
    bool processed;
    int64_t created_at;
    int64_t updated_at;
};
class Upload
{
public:
    int id;
    std::string path;
    std::string remoteID;
    int progress;
    Status status;
    std::string error;
    int64_t created_at;
    int64_t updated_at;
};
class Download
{
public:
    int id;
    std::string remoteID;
    std::string link;
    int progress;
    Status status;
    std::string error;
    int64_t created_at;
    int64_t updated_at;
};
class Runnable
{
public:
    int id;
    std::string remoteID;
    bool showWindow;
    bool run;
    std::string name;
    std::string link;
    Status status;
    int64_t created_at;
    int64_t updated_at;
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
    MC_CONNECTIONS
};
class Message
{
public:
    MessageCode code;
    std::string recepient;
    std::string sender;
    std::string data;
};