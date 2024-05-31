#pragma once
#include <winsock2.h>
// #include "Websocks.h"
#include "Utils.h"
#include <mutex>
#include "Models.h"
#include <functional>
#include <nlohmann/json.hpp>
#include "rtc/websocket.hpp"
typedef std::function<void(Message)> SocketMessageHandler;
typedef std::function<void(bool)> ConnectionChangeHandler;
class MessageHandler
{
private:
    rtc::WebSocket ws;
    std::mutex _msgLck;
    void _onMessage(const std::string &msg);

public:
    std::atomic_bool connected;
    void _handleMessage(Message msg);
    void _connectTask();
    std::atomic_bool running;
    SocketMessageHandler messageHandler;
    ConnectionChangeHandler connectionChangeHandler;

    bool MessageHandler::sendData(std::string recepient, MessageCode code, std::string data, bool compressed = false);
    bool MessageHandler::sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data,std::string meta, bool compressed = false);
    MessageHandler();
};
extern MessageHandler *defaultMessageHandler;