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