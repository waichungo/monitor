#pragma once
#include <winsock2.h>
#include "Websocks.h"
#include "Utils.h"
#include "Models.h"

class MessageHandler
{
    private:
    WebsocketManager wsManager;
public:
    void MessageHandler::sendData(std::string recepient, MessageCode code,std::string data);
     MessageHandler();
};
 static MessageHandler defaultMessageHandler;