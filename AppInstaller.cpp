#include "AppInstaller.h"
#include "Base64.hpp"
#include "ZipExt.h"
#include "Models.h"
#include "Downloader.h"
#include "DB.h"


AppInstallResult InstallApp(Runnable runnable)
{
    AppInstallResult res;
    res.success = false;
    std::string file = (fs::path(GetDefaultAppsDownloadsDir()) /= runnable.remoteID).string();
    if (!Exists(file))
    {
        file += "." + getFileExtension(runnable.name);
    }
    if (Exists(file))
    {
        auto dir = fs::path(GetAppsDir()) /= runnable.remoteID;
        res.installDir = dir.string();
        if (!Exists(dir.string()))
        {
            try
            {
                fs::create_directories(dir);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        std::string target;
        if (StringUtils::endsWith(runnable.name, ".exe"))
        {
            target = (dir /= fs::path(runnable.name).filename()).string();
            res.mainExe = target;
            auto l = GetFileLock(target);
            while (!l.Lock())
            {
                Sleep(200);
            }
            if (Exists(target))
            {
                KillAllFromDirectory(dir.string());
                try
                {
                    fs::remove(target);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            bool moved = MoveFileA(file.c_str(), target.c_str());
            res.success = moved;
            return res;
        }
        else if (StringUtils::endsWith(runnable.name, ".zip"))
        {
            auto l = GetFileLock(dir.string());
            while (!l.Lock())
            {
                Sleep(200);
            }
            KillAllFromDirectory(dir.string());
            try
            {
                for (auto &path : fs::recursive_directory_iterator(dir.string()))
                {
                    try
                    {
                        if (!path.is_directory())
                        {

                            fs::remove(path);
                        }
                        else
                        {
                            fs::remove_all(dir.string());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            bool extracted = zipext::ExtractToFolder(file, dir.string());
            res.success = extracted;
            if (extracted)
            {
                try
                {
                    fs::remove(file);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            auto dirTmp = dir;

            auto appName = runnable.name;
            if (StringUtils::endsWith(appName, ".zip"))
            {
                appName = RemoveExt(appName) + ".exe";
            }
            target = (dirTmp /= appName).string();
            if (Exists(target))
            {
                res.mainExe = target;
            }
            else
            {
                dirTmp = dir;
                for (auto &file : fs::recursive_directory_iterator(dirTmp))
                {
                    if (StringUtils::endsWith(file.path().string(), ".exe"))
                    {
                        res.mainExe = file.path().string();
                        break;
                    }
                }
            }
            return res;
        }
    }
    else
    {
        res.success = false;
    }
    return res;
}