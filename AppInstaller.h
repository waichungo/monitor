#pragma once
#include <winsock2.h>
#include "Utils.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <intrin.h>
#include "App.h"
#include "stringutils.h"

#include "thread"
#include "memory"
#include "Request.h"
#include "Models.h"
#include "Locker.h"
typedef struct AppInstallResult
{
    std::string installDir;
    std::string mainExe;
    bool success;
} AppInstallResult;
AppInstallResult InstallApp(Runnable runnable);