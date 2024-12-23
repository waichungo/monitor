#pragma once
#include <winsock2.h>
#include "Utils.h"
#include "Models.h"
#include "App.h"
#include <map>
#include <mutex>
#include "App.h"
class MessageProcessor
{
public:
    static void processMessage(Message msg);
};

class DataPartManager
{
private:
public:
    DataPartManager();
    // DataPartManager()=delete ;
    std::vector<uint8_t> data;
    MessageCode kind;
    std::string id;
    bool compressed;
    int64_t lastInteraction;
    DataPartManager(std::vector<uint8_t> payload, MessageCode internalType, bool isCompressed, std::string identifier = generate_uuid_v4());
};
class FilePartManager
{
private:
    FILE *file;
    int64_t position;
    MessageCode kind = MC_FILE_TRANSFER;
    // mutable std::mutex _lck;

public:
    FilePartManager();
    int64_t size;
    // DataPartManager()=delete ;

    std::string path;
    std::string id;
    int64_t lastInteraction;
    int64_t written();
    std::vector<uint8_t> Read(int start, int totalRead);
    ~FilePartManager();
    FilePartManager(std::string path, std::string id);
};
typedef std::function<void(std::map<std::string, DataPartManager> *)> DataMapAccessorFunc;
class SafeDataPartQueueAccessor
{

private:
    std::mutex _lck;
    std::map<std::string, DataPartManager> queue;

public:
    SafeDataPartQueueAccessor();
    void Access(DataMapAccessorFunc dn);
    size_t size();
};
typedef std::function<void(std::map<std::string, FilePartManager> *)> FileMapAccessorFunc;
class SafeFilePartQueueAccessor
{

private:
    std::mutex _lck;
    std::map<std::string, FilePartManager> queue;

public:
    SafeFilePartQueueAccessor();
    void Access(FileMapAccessorFunc dn);
    size_t size();
};
extern SafeDataPartQueueAccessor dataPartsAccessor;
extern SafeFilePartQueueAccessor filePartsAccessor;

class PartMeta
{
public:
    PartMeta() = default;
    virtual ~PartMeta() = default;

private:
    std::string id;
    int64_t start;
    int64_t type;
    int64_t size;
    int64_t request_size;
    bool compress;
    bool is_file;
    bool internally_compressed;
    std::string arg;

public:
    const std::string &get_id() const { return id; }
    std::string &get_mutable_id() { return id; }
    void set_id(const std::string &value) { this->id = value; }

    const int64_t &get_start() const { return start; }
    int64_t &get_mutable_start() { return start; }
    void set_start(const int64_t &value) { this->start = value; }

    const int64_t &get_type() const { return type; }
    int64_t &get_mutable_type() { return type; }
    void set_type(const int64_t &value) { this->type = value; }

    const int64_t &get_size() const { return size; }
    int64_t &get_mutable_size() { return size; }
    void set_size(const int64_t &value) { this->size = value; }

    const int64_t &get_request_size() const { return request_size; }
    int64_t &get_mutable_request_size() { return request_size; }
    void set_request_size(const int64_t &value) { this->request_size = value; }

    const bool &get_compress() const { return compress; }
    bool &get_mutable_compress() { return compress; }
    void set_compress(const bool &value) { this->compress = value; }

    const bool &get_is_file() const { return is_file; }
    bool &get_mutable_is_file() { return is_file; }
    void set_is_file(const bool &value) { this->is_file = value; }

    const bool &get_internally_compressed() const { return internally_compressed; }
    bool &get_mutable_internally_compressed() { return internally_compressed; }
    void set_internally_compressed(const bool &value) { this->internally_compressed = value; }

    const std::string &get_arg() const { return arg; }
    std::string &get_mutable_arg() { return arg; }
    void set_arg(const std::string &value) { this->arg = value; }

    static PartMeta from_json(std::string data)
    {
        PartMeta meta;
        json j = json::parse(data);
        meta.set_id(j.contains("id") ? j.at("id").get<std::string>() : "");
        meta.set_start(j.contains("start") ? j.at("start").get<int64_t>() : 0);
        meta.set_type(j.contains("type") ? j.at("type").get<int64_t>() : 0);
        meta.set_size(j.contains("size") ? j.at("size").get<int64_t>() : 0);
        meta.set_request_size(j.contains("requestSize") ? j.at("requestSize").get<int64_t>() : 0);
        meta.set_compress(j.contains("compress") ? j.at("compress").get<bool>() : false);
        meta.set_internally_compressed(j.contains("internallyCompressed") ? j.at("internallyCompressed").get<bool>() : false);
        meta.set_arg(j.contains("arg") ? j.at("arg").get<std::string>() : "");
        meta.set_is_file(j.contains("isFile") ? j.at("isFile").get<bool>() : false);
        return meta;
    }

    json to_json()
    {
        json j = json::object();
        j["id"] = get_id();
        j["start"] = get_start();
        j["type"] = get_type();
        j["size"] = get_size();
        j["requestSize"] = get_request_size();
        j["compress"] = get_compress();
        j["internallyCompressed"] = get_internally_compressed();
        j["isFile"] = get_is_file();
        j["arg"] = get_arg();
        return j;
    }
};
void partCleanerTask();
void EnqueueDataPart(DataPartManager &part, std::string recepient, int startIndex, bool compress = false, int partSize = 1024 * 20);
void EnqueueFilePart(FilePartManager &part, std::string recepient, int startIndex, bool compress = true, int partSize = 1024 * 50);