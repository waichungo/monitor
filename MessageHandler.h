#pragma once
#include "Utils.h"
#include "Models.h"

class MessageHandler
{
public:
    void MessageHandler::sendData(std::string recepient, MessageCode code,std::string data);
};
 static MessageHandler defaultMessageHandler;