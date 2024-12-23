#include "Uploader.h"
#include "Utils.h"
#include "Models.h"
#include "curl/curl.h"
#include "App.h"
#include "DB.h"
class ReadData
{
public:
    const char *data;
    size_t length;
    bool sentFile;
};
class ExtFileBag
{
public:
    FILE *file;
    std::atomic_bool *stopSignal;
    ProgressData *progressData;
    ReadData *readData;
};

size_t minimum(size_t num1, size_t num2)
{
    return num1 < num2 ? num1 : num2;
}

size_t responseWriteCallback(void *pContents, size_t size, size_t nmemb, void *pUser)
{
    ((std::string *)pUser)->append((char *)pContents, size * nmemb);
    return size * nmemb;
}
int uploadProgressCallback(void *ptr, double TotalToDownload, double NowDownloaded,
                           double TotalToUpload, double NowUploaded)
{
    FileBag *bag = (FileBag *)ptr;
    if (bag->progressData)
    {
        ProgressData *arg = bag->progressData;
        arg->transferred = (int64_t)NowUploaded;
        arg->size = (int64_t)TotalToUpload;
    }
    if (bag->stopSignal && *bag->stopSignal)
    {
        return -1;
    }
    // if you don't return 0, the transfer will be aborted - see the documentation
    return 0;
}
int gdriveUploadProgressCallback(void *ptr,
                                 curl_off_t dltotal,
                                 curl_off_t dlnow,
                                 curl_off_t ultotal,
                                 curl_off_t ulnow)
{
    FileBag *bag = (FileBag *)ptr;
    if (bag->progressData)
    {
        ProgressData *arg = bag->progressData;
        arg->transferred = (int64_t)ulnow;
        arg->size = ultotal > 0 ? (int64_t)ultotal : bag->progressData->size;
    }
    if (bag->stopSignal && *bag->stopSignal)
    {
        return -1;
    }
    // if you don't return 0, the transfer will be aborted - see the documentation
    return 0;
}
// size_t gDriveReadCallback(void *ptr, size_t size, size_t nmemb, void *stream)
// {
//     std::ifstream *ifs = static_cast<std::ifstream *>(stream);
//     if (!ifs->eof())
//     {
//         ifs->read(static_cast<char *>(ptr), size * nmemb);
//         return ifs->gcount();
//     }
//     return 0;
// }
size_t readCallback(void *ptr, size_t size, size_t nitems, void *data)
{
    FileBag *bag = (FileBag *)data;
    size_t bytes_read;
    bytes_read = fread(ptr, 1, (size * nitems), bag->file);
    if (bag->progressData != nullptr)
    {
        if (bag->stopSignal != nullptr && *bag->stopSignal)
        {
            return CURL_READFUNC_PAUSE;
        }
        bag->progressData->transferred += bytes_read;
    }

    return bytes_read;
}
// size_t gDriveReadCallback(void *ptr, size_t size, size_t nitems, void *data)
// {
//     ExtFileBag *bag = (ExtFileBag *)data;
//     size_t bytes_read;
//     bytes_read = fread(ptr, 1, (size * nitems), bag->file);
//     if (bag->progressData != nullptr)
//     {
//         if (bag->stopSignal != nullptr && *bag->stopSignal)
//         {
//             return CURL_READFUNC_PAUSE;
//         }
//         bag->progressData->transferred += bytes_read;
//     }

//     return bytes_read;
// }
// size_t mimeReadCallback(char *buffer, size_t size, size_t nitems, void *arg){
//     FileBag *bag = (FileBag *)data;
//     size_t bytes_read;
//     bytes_read = fread(ptr, 1, (size * nitems), bag->file);
//     if (bag->progressData != nullptr)
//     {
//         if (bag->stopSignal != nullptr && *bag->stopSignal)
//         {
//             return CURL_READFUNC_PAUSE;
//         }
//         bag->progressData->transferred += bytes_read;
//     }

//     return bytes_read;
// }
UploadResponse uploadAppwriteFile(std::string file, app::AppwriteDrive drive, std::atomic_bool *stopSignal, ProgressFunc progressCallback)
{
    CURL *curl = curl_easy_init();

    std::map<std::string, std::string> bodyFields;
    std::map<std::string, std::string> headers;

    headers.emplace(OBFUSCATED("X-Appwrite-Key"), drive.credential);
    headers.emplace(OBFUSCATED("X-Appwrite-Project"), drive.project_id);
    headers.emplace(OBFUSCATED("X-Appwrite-Response-Format"), "1.4.0");

    auto id = generate_uuid_v4();

    bodyFields.emplace("fileId", id);

    std::string link = OBFUSCATED("https://cloud.appwrite.io/v1/storage/buckets/");
    link += drive.bucket_id;
    link += "/files";

    return uploadFile(file, link, bodyFields, headers, stopSignal, progressCallback);
}
UploadResponse uploadGoogleDriveFile(std::string file, app::GoogleDrive drive, std::atomic_bool *stopSignal, ProgressFunc progressCallback)
{
    CURL *curl;
    CURLcode res;
    UploadResponse resp;
    resp.code = 0;
    resp.success = false;

    curl = curl_easy_init();

    if (curl)
    {
        std::string accessToken = GetGDriveToken(drive.get_id());
        if (accessToken.empty())
        {
            resp.success = false;
            resp.error = OBFUSCATED("Failed to obtain google drive access token");
            return resp;
        }
        struct curl_slist *headers = NULL;
        size_t size = GetFileSize(file);
        std::string fileName = fs::path(file).filename().string();
        long responseCode = 0;
        std::string metadata = "{\"name\": \"" + fileName + "\"}";

        ProgressData arg;
        arg.size = size;
        arg.name = fs::path(file).filename().string();
        arg.transferred = 0;
        arg.eta = 0;

        FileBag bag;
        bag.progressData = &arg;
        bag.stopSignal = stopSignal;

        std::string response;

        std::string url = OBFUSCATED("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");

        curl_httppost *formpost = NULL;
        curl_httppost *lastptr = NULL;

        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_COPYNAME, OBFUSCATED("metadata"),
                     CURLFORM_COPYCONTENTS, metadata.c_str(),
                     CURLFORM_CONTENTTYPE, OBFUSCATED("application/json; charset=UTF-8"),
                     CURLFORM_END);
        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_COPYNAME, "file",
                     CURLFORM_FILE, file.c_str(),
                     CURLFORM_CONTENTTYPE, "*/*",
                     CURLFORM_END);
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the HTTP method to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set the authorization header
        std::string authHeader = OBFUSCATED("Authorization: Bearer ");
        authHeader += accessToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, uploadProgressCallback);
        // curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &bag);

        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &bag);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, gdriveUploadProgressCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        std::shared_ptr<std::thread> progressThread;
        std::atomic_bool stopProgress = false;
        if (progressCallback != nullptr)
        {
            std::thread pThread((std::function<void()>)[&]() {
                int64_t lastTransfer = -1;
                while (!stopProgress)
                {
                    for (size_t i = 0; i < 50; i++)
                    {
                        if (stopProgress)
                        {
                            break;
                        }
                        Sleep(100);
                    }
                    if (lastTransfer < arg.transferred)
                    {
                        progressCallback(arg);
                        lastTransfer = arg.transferred;
                    }
                }
            });
            progressThread = std::make_shared<std::thread>(std::move(pThread));
        }

        // Perform the request
        res = curl_easy_perform(curl);
        resp.response = response;
        stopProgress = true;
        if (progressThread && progressThread->joinable())
        {
            progressThread->join();
        }
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        resp.code = http_code;
        resp.response = response;
        if (res == CURLE_OK && http_code >= 200 && http_code < 300)
        {
            resp.success = true;
        }
        else
        {
            resp.success = false;
            if (res != CURLE_OK)
            {
                std::string error = curl_easy_strerror(res);
                resp.error = OBFUSCATED("Curl code error: ") + error;
            }
            else
            {
                resp.error = OBFUSCATED("Status code error: ") + std::to_string(http_code);
            }
        }
        curl_formfree(formpost);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
    {
        resp.success = false;
        resp.error = OBFUSCATED("Failed to initialize libcurl");
    }

    return resp;
}
// UploadResponse uploadGoogleDriveFile(std::string file, app::GoogleDrive drive, std::atomic_bool *stopSignal, ProgressFunc progressCallback)
// {
//     UploadResponse resp;
//     resp.code = 0;
//     resp.success = false;
//     CURL *curl;
//     CURLcode res;
//     struct curl_slist *headers = NULL;

//     FILE *stream = fopen(file.c_str(), "r");
//     if (stream == NULL)
//     {
//         resp.error = OBFUSCATED("Failed to open file for upload");
//         resp.success = false;
//         return resp;
//     }

//     std::string url = OBFUSCATED("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");
//     std::string fileName = fs::path(file).filename().string();
//     std::string metadata = "{\"name\": \"" + fileName + "\"}";

//     std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
//     std::string metadataPart = "--" + boundary + "\r\n"
//                                                  "Content-Type: application/json; charset=UTF-8\r\n\r\n" +
//                                metadata + "\r\n--" + boundary + "\r\n"
//                                                                 "Content-Type: application/octet-stream\r\n\r\n";
//     std::string fileHeader = "--" + boundary + "\r\n"
//                                                "Content-Disposition: form-data; name=\"file\"; filename=\"" +
//                              fileName + "\"\r\n"
//                                         "Content-Type: application/octet-stream\r\n\r\n";

//     std::string endPart = "\r\n--" + boundary + "--";

//     size_t size = GetFileSize(file);
//     long responseCode = 0;

//     ProgressData arg;
//     arg.size = size;
//     arg.name = fs::path(file).filename().string();
//     arg.transferred = 0;
//     arg.eta = 0;

//     ReadData

//     ExtFileBag bag;
//     bag.file = stream;
//     bag.progressData = &arg;
//     bag.stopSignal = stopSignal;

//     std::string response;

//     std::string accessToken = GetGDriveToken(drive.get_id());
//     if (!accessToken.empty())
//     {
//         curl = curl_easy_init();
//         if (curl)
//         {
//             bag.progressData = &arg;
//             bag.stopSignal = stopSignal;
//             std::atomic_bool stopProgress = false;

//             std::shared_ptr<std::thread> progressThread;

//             if (progressCallback != nullptr)
//             {
//                 std::thread pThread((std::function<void()>)[&]() {
//                     int64_t lastTransfer = 0;
//                     while (!stopProgress)
//                     {
//                         for (size_t i = 0; i < 50; i++)
//                         {
//                             if (stopProgress)
//                             {
//                                 break;
//                             }
//                             Sleep(100);
//                         }
//                         if (lastTransfer < arg.transferred)
//                         {
//                             progressCallback(arg);
//                             lastTransfer = arg.transferred;
//                         }
//                     }
//                 });
//                 progressThread = std::make_shared<std::thread>(std::move(pThread));
//             }

//             curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

//             curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseWriteCallback);
//             curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

//             // Set the authorization header
//             std::string authHeader = OBFUSCATED("Authorization: Bearer ") + accessToken;
//             headers = curl_slist_append(headers, authHeader.c_str());

//             std::string contentTypeHeader = "Content-Type: multipart/related; boundary=" + boundary;
//             headers = curl_slist_append(headers, contentTypeHeader.c_str());

//             std::string fullPostData = metadataPart + fileHeader;
//             curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fullPostData.size() + endPart.size());

//             curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

//             // Set the read callback function and the file stream
//             curl_easy_setopt(curl, CURLOPT_READFUNCTION, gDriveReadCallback);
//             curl_easy_setopt(curl, CURLOPT_READDATA, &bag);

//             // Build the multipart request in chunks to avoid loading the entire file into memory
//             curl_mime *mime;
//             curl_mimepart *part;

//             mime = curl_mime_init(curl);

//             // Add metadata part
//             part = curl_mime_addpart(mime);
//             curl_mime_data(part, metadataPart.c_str(), CURL_ZERO_TERMINATED);

//             // Add file part
//             part = curl_mime_addpart(mime);
//             // curl_mime_data_cb(part, CURL_ZERO_TERMINATED, readCallback, nullptr, nullptr, &fileStream);
//             curl_mime_type(part, "application/octet-stream");
//             // curl_mime_type(part, "*/*");

//             // Add ending boundary
//             part = curl_mime_addpart(mime);
//             curl_mime_data(part, endPart.c_str(), CURL_ZERO_TERMINATED);

//             curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

//             curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, uploadProgressCallback);
//             curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &bag);

//             std::string postData = metadataPart + fileHeader;
//             curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
//             curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postData.size());

//             // Perform the request
//             res = curl_easy_perform(curl);
//             stopProgress = true;
//             if (progressThread && progressThread->joinable())
//             {
//                 progressThread->join();
//             }
//             long http_code = 0;
//             curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

//             resp.code = http_code;
//             resp.response = response;
//             if (res == CURLE_OK && http_code >= 200 && http_code < 300)
//             {
//                 resp.success = true;
//             }
//             else
//             {
//                 resp.success = false;
//                 if (res != CURLE_OK)
//                 {
//                     std::string error = curl_easy_strerror(res);
//                     resp.error = OBFUSCATED("Curl code error: ") + error;
//                 }
//                 else
//                 {
//                     resp.error = OBFUSCATED("Status code error: ") + std::to_string(http_code);
//                 }
//             }
//             // Cleanup
//             curl_mime_free(mime);
//             curl_slist_free_all(headers);
//             curl_easy_cleanup(curl);
//         }
//         else
//         {
//             resp.error = OBFUSCATED("Failed to initialize curl");
//             resp.success = false;
//             return resp;
//         }
//         if (stream)
//         {
//             fclose(stream);
//         }
//     }
//     else
//     {
//         resp.error = OBFUSCATED("Failed to get access token");
//         resp.success = false;
//         return resp;
//     }
//     return resp;
// }
// UploadResponse uploadGoogleDriveFile(std::string file, app::GoogleDrive drive, std::atomic_bool *stopSignal, ProgressFunc progressCallback)
// {
//     CURL *curl = curl_easy_init();

//     std::map<std::string, std::string> headers;

//     std::string accessToken = GetGDriveToken(drive.get_id());
//     std::string tokenHeaderPart = OBFUSCATED("Bearer ");
//     tokenHeaderPart += accessToken;
//     headers.emplace(OBFUSCATED("Authorization"), tokenHeaderPart);
//     // https://www.googleapis.com/upload/drive/v3/files
//     std::string link = OBFUSCATED("https://www.googleapis.com/upload/drive/v3/files?uploadType=media");

//     return uploadFile(file, link,  {}, headers, stopSignal, progressCallback);
// }
UploadResponse uploadFile(std::string file, std::string uploadLink, std::map<string, string> bodyFields, std::map<string, string> headers, std::atomic_bool *stopSignal, ProgressFunc progressCallback)
{
    UploadResponse resp;
    resp.code = 0;
    resp.success = false;
    CURL *curl = curl_easy_init();
    struct curl_slist *headerlist = NULL;

    size_t size = GetFileSize(file);
    long responseCode = 0;

    curl_httppost *post = NULL;
    curl_httppost *last = NULL;

    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, file.c_str(),
                 CURLFORM_END);

    for (auto &field : bodyFields)
    {
        curl_formadd(&post, &last,
                     CURLFORM_COPYNAME, field.first.c_str(),
                     CURLFORM_COPYCONTENTS, field.second.c_str(),
                     CURLFORM_END);
    }

    for (auto &header : headers)
    {
        auto concatHeader = header.first + ": " + header.second;
        headerlist = curl_slist_append(headerlist, concatHeader.c_str());
    }
    // FILE *stream = fopen(file.c_str(), "r");
    // if (stream == NULL)
    // {
    //     resp.error = OBFUSCATED("Failed to open file for upload");
    //     resp.success = false;
    //     return resp;
    // }
    ProgressData arg;
    arg.size = size;
    arg.name = fs::path(file).filename().string();
    arg.transferred = 0;
    arg.eta = 0;

    FileBag bag;
    // bag.file = stream;
    bag.progressData = &arg;
    bag.stopSignal = stopSignal;
    std::atomic_bool stopProgress = false;

    std::shared_ptr<std::thread> progressThread;

    if (progressCallback != nullptr)
    {
        std::thread pThread((std::function<void()>)[&]() {
            int64_t lastTransfer = 0;
            while (!stopProgress)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    if (stopProgress)
                    {
                        break;
                    }
                    Sleep(100);
                }
                if (lastTransfer < arg.transferred)
                {
                    progressCallback(arg);
                    lastTransfer = arg.transferred;
                }
            }
        });
        progressThread = std::make_shared<std::thread>(std::move(pThread));
    }

    curl_easy_setopt(curl, CURLOPT_URL, uploadLink.c_str());
    // curl_easy_setopt(curl, CURLOPT_READDATA, &file);
    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    // Set the read callback function

    // Set the file size
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);
    if (headerlist)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    std::string response;

    // curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    // curl_easy_setopt(curl, CURLOPT_READDATA, &bag);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    // curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3600L);
    // curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    // Install the callback function
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, uploadProgressCallback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &bag);

    // curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

    auto res = curl_easy_perform(curl);
    // fclose(stream);
    stopProgress = true;
    if (progressThread && progressThread->joinable())
    {
        progressThread->join();
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (headerlist)
    {
        curl_slist_free_all(headerlist);
    }
    resp.code = http_code;
    resp.response = response;
    if (res == CURLE_OK && http_code >= 200 && http_code < 300)
    {
        resp.success = true;
    }
    else
    {
        resp.success = false;
        if (res != CURLE_OK)
        {
            std::string error = curl_easy_strerror(res);
            resp.error = OBFUSCATED("Curl code error: ") + error;
        }
        else
        {
            resp.error = OBFUSCATED("Status code error: ") + std::to_string(http_code);
        }
    }
    curl_easy_cleanup(curl);
    return resp;
}

// size_t gUploadReadCallback(void *ptr, size_t size, size_t nmemb, void *stream)
// {
//     std::ifstream *ifs = static_cast<std::ifstream *>(stream);
//     ifs->read(static_cast<char *>(ptr), size * nmemb);
//     return ifs->gcount();
// }

// Function to upload file to Google Drive
void uploadFileToGoogleDrive(std::string file, app::GoogleDrive drive)
{
    auto accessToken = GetGDriveToken(drive.get_id());
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    std::string response;
    std::string fileName = fs::path(file).filename().string();

    std::string url = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    std::string metadata = "{\"name\": \"" + fileName + "\"}";

    curl = curl_easy_init();
    if (curl)
    {
        curl_httppost *formpost = NULL;
        curl_httppost *lastptr = NULL;

        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_COPYNAME, OBFUSCATED("metadata"),
                     CURLFORM_COPYCONTENTS, metadata.c_str(),
                     CURLFORM_CONTENTTYPE, OBFUSCATED("application/json; charset=UTF-8"),
                     CURLFORM_END);
        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_COPYNAME, "file",
                     CURLFORM_FILE, file.c_str(),
                     CURLFORM_CONTENTTYPE, "*/*",
                     CURLFORM_END);
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the HTTP method to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set the authorization header
        std::string authHeader = OBFUSCATED("Authorization: Bearer ");
        authHeader += accessToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        // Function to read the full POST data including file content and end boundary
        // struct ReadData
        // {
        //     const char *data;
        //     size_t length;
        //     std::ifstream *fileStream;
        //     bool sentFile;
        //     std::string endBoundary;
        // } readData = {metadataPart.c_str(), metadataPart.size(), &fileStream, false, endPart};

        // auto customReadCallback = [](char *buffer, size_t size, size_t nitems, void *userdata) -> size_t
        // {
        //     ReadData *rd = static_cast<ReadData *>(userdata);
        //     size_t bufferSize = size * nitems;
        //     if (rd->length > 0)
        //     {
        //         size_t toCopy = minimum(bufferSize, rd->length);
        //         std::memcpy(buffer, rd->data, toCopy);
        //         rd->data += toCopy;
        //         rd->length -= toCopy;
        //         return toCopy;
        //     }
        //     if (!rd->sentFile)
        //     {
        //         size_t bytesRead = gUploadReadCallback(buffer, size, nitems, rd->fileStream);
        //         if (bytesRead == 0)
        //         {
        //             rd->sentFile = true;
        //         }
        //         return bytesRead;
        //     }
        //     if (rd->sentFile && rd->length == 0)
        //     {
        //         rd->data = rd->endBoundary.c_str();
        //         rd->length = rd->endBoundary.size();
        //         return 0; // return 0 here to let curl call us again
        //     }
        //     if (rd->length > 0)
        //     {
        //         size_t toCopy = minimum(bufferSize, rd->length);
        //         std::memcpy(buffer, rd->data, toCopy);
        //         rd->data += toCopy;
        //         rd->length -= toCopy;
        //         return toCopy;
        //     }
        //     return 0;
        // };

        // curl_easy_setopt(curl, CURLOPT_READFUNCTION, customReadCallback);
        // curl_easy_setopt(curl, CURLOPT_READDATA, &readData);

        // Perform the request
        res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            std::cout << "File uploaded successfully!" << std::endl;
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    // fileStream.close();
}
std::shared_ptr<app::UploadProgress> createOrFindUploadProgressForUpload(app::Upload &upload)
{
    std::shared_ptr<app::UploadProgress> prog;
    DBValue val;

    val.stringValue = std::make_shared<std::string>(upload.get_id());
    std::map<std::string, DBValue> mp;
    mp.emplace("id", val);

    auto progs = findUploadProgresses(mp, 1);
    if (!progs.empty())
    {
        prog = std::make_shared<app::UploadProgress>(progs[0]);
    }
    else
    {
        auto tmpProg = app::UploadProgress();
        tmpProg.complete = false;
        tmpProg.machine_id = GetMachineID();
        tmpProg.state = UPLOADSTATUS::US_UPLOADING;
        tmpProg.size = (int64_t)GetFileSize(upload.path);
        tmpProg.doneWithUpdate = false;
        tmpProg.drive_id = upload.drive_id;
        tmpProg.eta = 0;
        tmpProg.type = upload.type;
        tmpProg.uploaded = 0;
        tmpProg.stopped = false;
        tmpProg.rate = 0;
        tmpProg.id = upload.id;
        tmpProg.path = upload.path;
        tmpProg.doneWithUpdate = false;
        prog = SaveUploadProgress(tmpProg);
    }
    return prog;
}
void uploadProgressCallbackForUpload(app::Upload upload, ProgressData prog)
{
    auto progress = createOrFindUploadProgressForUpload(upload);
    progress->uploaded = prog.transferred;
    UpdateUploadProgress(*progress);
}
void uploadUpload(app::Upload upload)
{
    if (upload.completed || upload.stopped)
        return;
    std::string mtx = OBFUSCATED("Upload__") + upload.get_id();
    // mtx += OBFUSCATED("downloader_task");
    Locker lck(mtx, true);
    if (lck.Lock())
    {
        WaitForConnection();

        UploadResponse resp;
        std::atomic_bool stopDownload = false;
        std::atomic_bool exitMonitor = false;

        std::thread monitorThread((std::function<void()>)[&]() {
            while (!exitMonitor)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    Sleep(100);
                    if (exitMonitor)
                    {
                        break;
                    }
                }
                auto lUp = findUpload(upload.local_id);
                if (lUp)
                {
                    if (lUp->stopped)
                    {
                        stopDownload.store(true);
                        break;
                    }
                }
            }
        });
        if (upload.get_type() == DriveKind::DR_GOOGLE)
        {
            auto gDrive = fetchGoogleDrive(upload.drive_id);
            if (gDrive != nullptr)
            {
                resp = uploadGoogleDriveFile(upload.path, *gDrive, &stopDownload, [&](ProgressData prog)
                                             { uploadProgressCallbackForUpload(upload, prog); });
            }
        }
        else if (upload.get_type() == DriveKind::DR_APPWRITE)
        {
            auto appDrive = fetchAppwriteDrive(upload.drive_id);
            if (appDrive != nullptr)
            {
                resp = uploadAppwriteFile(upload.path, *appDrive, &stopDownload, [&](ProgressData prog)
                                          { uploadProgressCallbackForUpload(upload, prog); });
            }
        }

        exitMonitor = true;
        if (monitorThread.joinable())
        {
            monitorThread.join();
        }
        auto progress = createOrFindUploadProgressForUpload(upload);

        progress->complete = resp.success;
        progress->error = resp.error;
        progress->state = stopDownload ? UPLOADSTATUS::US_STOPPED : (resp.success ? UPLOADSTATUS::US_COMPLETE : UPLOADSTATUS::US_ERROR);
        
        UpdateUploadProgress(*progress);

        if(resp.success){
            auto lup=findUpload(upload.local_id);
            if(lup){
                lup->completed=true;
                UpdateUpload(*lup);                
            }
        }
    }
}