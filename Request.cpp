#include "Request.h"

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    app::HttpResponse *resp = (app::HttpResponse *)data;
    size_t prevSz = resp->data.size();
    size_t realsize = size * nmemb;
    resp->data.resize(prevSz + realsize);
    memcpy(&resp->data.data()[prevSz], ptr, realsize);
    // memcpy(ptr, &resp->data.data()[prevSz], realsize);
    return realsize;
}
app::HttpResponse GetBytesFromURL(string url)
{
    std::string USERAGENT = OBFUSCATED("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4573.0 Safari/537.36");
    app::HttpResponse resp{0};

    CURL *curl_handle;
    CURLcode res;

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */

    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    // #ifdef _DEBUG
    //     curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
    // #endif

    /* send all data to this function  */

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    // curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&resp);

    /* some servers do not like requests that are made without a user-agent
       field, so we provide one */

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USERAGENT.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
    auto response = curl_easy_perform(curl_handle);

    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    resp.status = http_code;
    curl_off_t cl;
    curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
    resp.total = cl;
    curl_easy_cleanup(curl_handle);
    return resp;
}