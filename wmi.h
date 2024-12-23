#pragma once
#include <map>
#include <string>
#include <vector>

class WMIInfo{
    public:
    std::map<std::string,std::string> cpuInfo;
    std::map<std::string,std::string> osInfo;
    std::map<std::string,std::string> diskInfo;
};

WMIInfo getWmiInformation();