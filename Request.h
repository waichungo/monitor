#pragma once

#include <curl/curl.h>

#include <vector>
#include <string>
#include <map>
#include "MetaString.h"
using namespace andrivet::ADVobfuscator;
using std::string;
using std::vector;
namespace app{
typedef struct HttpResponse
{
    int status;
    vector<unsigned char> data;
    long total=0;
} _HttpResponse;
}
app::HttpResponse GetBytesFromURL(string url,std::map<std::string,std::string> headers=std::map<std::string,std::string>());