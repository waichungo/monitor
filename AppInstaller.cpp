#include "AppInstaller.h"
#include "Base64.hpp"
#include "ZipExt.h"

std::string GetAppsDir()
{
    auto dir = fs::path(GetAssetDir()) /= OBFUSCATED("apps");
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
    return dir.string();
}
AppInstallResult InstallApp(std::string file)
{
    AppInstallResult res;
    res.success = false;
    if (Exists(file))
    {
        auto dir = fs::path(GetAppsDir()) /= RemoveExt(fs::path(file).filename().string());
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
        if (StringUtils::endsWith(file, ".exe"))
        {
            target = (dir /= fs::path(file).filename()).string();
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
        else if (StringUtils::endsWith(file, ".zip"))
        {
            auto l = GetFileLock(dir.string());
            while (!l.Lock())
            {
                Sleep(200);
            }
            KillAllFromDirectory(dir.string());
            for (auto &path : fs::recursive_directory_iterator(dir))
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
            bool extracted = zipext::ExtractToFolder(file, dir.string());
            res.success = extracted;
            if(extracted){
                try
                {
                    fs::remove(file);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
                
            }

            target = (dir /= (RemoveExt(fs::path(file).filename().string()) + ".exe")).string();
            if (Exists(target))
            {
                res.mainExe = target;
            }
            else
            {
                target = (dir /= (RemoveExt(fs::path(file).filename().string()) + ".dll")).string();
                if (Exists(target))
                {
                    res.mainExe = target;
                }
                else
                {
                    for (auto &file : fs::recursive_directory_iterator(dir))
                    {
                        if (StringUtils::endsWith(file.path().string(), ".exe"))
                        {
                            res.mainExe = file.path().string();
                            break;
                        }
                    }
                }
            }
            return res;
        }
    }
    return res;
}