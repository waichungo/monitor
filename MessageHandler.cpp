#include "MessageHandler.h"

void MessageHandler::sendData(std::string recepient, MessageCode code, std::string data)
{
    wsManager.sendData(recepient, code, data);
}
MessageHandler::MessageHandler()
{
    wsManager.setup();
}