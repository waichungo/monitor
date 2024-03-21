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
    std::shared_ptr<int> intValue;
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

std::shared_ptr<Command> SaveCommand(Command command);
std::shared_ptr<Command> findCommand(int64_t id);
std::vector<Command> findCommands(std::map<std::string, DBValue> params, int limit=200,std::string orderKey="updated_at",bool desc=true);
std::shared_ptr<Command> UpdateCommand(Command runnable);
bool deleteRunnable(int64_t id);