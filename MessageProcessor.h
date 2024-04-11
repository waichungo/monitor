#pragma once
#include <winsock2.h>
#include "Utils.h"
#include "Models.h"
#include "App.h"
class MessageProcessor{
    public:
        static void processMessage(Message msg);
};