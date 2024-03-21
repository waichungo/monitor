#include "App.h"
#include "DB.h"
#include "Models.h"

int main(int, char **)
{
    initializeDB();

    Runnable run;
    bool deleted = deleteRunnable(1);
    auto runnable = Runnable{
        10, generate_uuid_v4(), false, true, "executor2.exe", "https://app.com/executor.exe", Status::UNDEFINEDSTATUS, 0, 0};
    auto res = UpdateRunnable(runnable);
    std::map<std::string, DBValue> queryVals;
    DBValue dbVal;
    dbVal.equality = DBEquality::GREATERTHAN;
    dbVal.intValue = std::make_shared<int>(3);
    queryVals["id"] = dbVal;
    auto runnables = findRunnables(queryVals,200,"id");
    // uploadFile(std::string(R"(C:\Users\James\Downloads\AvaloniaVS.VS2022.zip)"));
    WaitForConnnection();
    Start();

    return 0;
}