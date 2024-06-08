#include "Uploader.h"
#include "Utils.h"
#include "Models.h"
#include "curl/curl.h"
size_t writeCallback(void *pContents, size_t size, size_t nmemb, void *pUser)
{
    ((std::string *)pUser)->append((char *)pContents, size * nmemb);
    return size * nmemb;
}

size_t readCallback(void *ptr, size_t size, size_t nitems, void *data)
{
    size_t bytes_read;

    /* I'm doing it this way to get closer to what the reporter is doing.
       Technically we don't need to do this, we could just use the default read
       callback which is fread. Also, 'size' param is always set to 1 by libcurl
       so it's fine to pass as buffer, size, nitems, instream. */
    bytes_read = fread(ptr, 1, (size * nitems), (FILE *)data);

    return bytes_read;
}
UploadResponse uploadAppwriteFile(std::string file, app::AppwriteDrive drive,std::atomic_bool* stopSignal,ProgressFunc progressCallback )
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

    return uploadFile(file, link, bodyFields, headers,stopSignal, progressCallback );
}
UploadResponse uploadFile(std::string file, std::string uploadLink, std::map<string, string> bodyFields, std::map<string, string> headers,std::atomic_bool* stopSignal,ProgressFunc progressCallback )
{
    UploadResponse resp;
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
    FILE *stream = fopen(file.c_str(), "r");
    if (stream == NULL)
    {
        resp.error = OBFUSCATED("Failed to open file for upload");
        resp.success = false;
        return resp;
    }
    ProgressData arg;
    
    FileBag bag;
    bag.file-stream;
    bag.progressData=&arg;
    bag.stopSignal=stopSignal;

    if(progressCallback!=nullptr){

    }

    curl_easy_setopt(curl, CURLOPT_URL, uploadLink.c_str());
    curl_easy_setopt(curl, CURLOPT_READDATA, &file);
    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    // Set the read callback function

    // Set the file size
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);
    if (headerlist)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    std::string response;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &stream);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(curl);
    fclose(stream);
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
            resp.error = "Curl code error: " + error;
        }
        else
        {
            resp.error = "Status code error: " + std::to_string(http_code);
        }
    }
    curl_easy_cleanup(curl);
    return resp;
}