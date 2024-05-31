#include "HttpUtil.h"
#include "App.h"
#include "map"
std::map<string, string> getDefaultHeaders()
{
    std::map<string, string> headers;
    headers["X-CLIENT"] = GetMachineID();
    headers["X-APP-TYPE"] = "client";
    return headers;
}
std::vector<app::Download> fetchDownloads(int64_t fromTime)
{
    std::vector<app::Download> res;
    std::string uri = SERVER_BASE + "/download";
    if (fromTime > 0)
    {
        uri += "?fromTime=" + std::to_string(fromTime);
    }
    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::PageResult pageRes;
            app::from_json(data.get_data(), pageRes);
            if (!pageRes.get_data().empty())
            {

                for (auto &el : pageRes.get_data())
                {
                    try
                    {
                        app::Download entry;
                        app::from_json(el, entry);
                        res.push_back(el);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
    }

    return res;
}
std::vector<app::Upload> fetchUploads(int64_t fromTime)
{
    std::vector<app::Upload> res;
    std::string uri = SERVER_BASE + "/upload";
    if (fromTime > 0)
    {
        uri += "?fromTime=" + std::to_string(fromTime);
    }
    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::PageResult pageRes;
            app::from_json(data.get_data(), pageRes);
            if (!pageRes.get_data().empty())
            {

                for (auto &el : pageRes.get_data())
                {
                    try
                    {
                        app::Upload entry;
                        app::from_json(el, entry);
                        res.push_back(el);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
    }

    return res;
}
std::vector<app::Command> fetchCommands(int64_t fromTime)
{
    std::vector<app::Command> res;
    std::string uri = SERVER_BASE + "/command";
    if (fromTime > 0)
    {
        uri += "?fromTime=" + std::to_string(fromTime);
    }
    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::PageResult pageRes;
            app::from_json(data.get_data(), pageRes);
            if (!pageRes.get_data().empty())
            {

                for (auto &el : pageRes.get_data())
                {
                    try
                    {
                        app::Command entry;
                        app::from_json(el, entry);
                        res.push_back(el);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
    }

    return res;
}
std::vector<app::CloudDrive> fetchDrives()
{
    std::vector<app::CloudDrive> res;
    std::string uri = SERVER_BASE + "/drives";

    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            for (auto &el : data.get_data())
            {
                try
                {
                    app::CloudDrive entry;
                    app::from_json(el, entry);
                    res.push_back(el);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }

    return res;
}
std::vector<app::CloudFile> fetchCloudApps(std::string googleDriveId, int64_t fromTime)
{
    std::vector<app::CloudFile> res;
    std::string uri = SERVER_BASE + "/gdrive/apps/list/";
    uri += googleDriveId;
    if (fromTime > 0)
    {
        uri += "?fromTime=" + std::to_string(fromTime);
    }
    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse resMessage;
        app::from_json(json::parse(resp.data), resMessage);
        if (resMessage.get_success())
        {
            auto data = resMessage.get_data();

            for (auto &el : data)
            {
                try
                {
                    app::CloudFile entry;
                    app::from_json(el, entry);
                    res.push_back(el);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }

    return res;
}