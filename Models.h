#include <string>
#include <vector>
enum CommandType
{
    UNDEFINED = 0,
    EXECUTESHELL,
    STOPAPP,
    UPLOADFILE,
    DOWNLOADFILE,
    DOWNLOADAPP
};
enum Status
{
    UNDEFINED = 0,
    FAILED,
    ERROR,
    PAUSED,
    STOPPED,
    COMPLETE
};

class Commmand
{
public:
    int id;
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
    bool showWindow;
    bool run;
    std::string name;
    Status status;
    int64_t created_at;
    int64_t updated_at;
};