#pragma once
// #include <winsock2.h>
// #include <mutex>
// #include "Models.h"
// #include "Utils.h"
// #include "MessageProcessor.h"
// #include "atomic"
// #include "rtc/websocket.hpp"
// // #include "WebSocketClient.h"
// // typedef NTSTATUS(__stdcall *NtQueryInformationProcessFunc)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
// typedef std::function<void(Message)> SocketMessageHandler;
// typedef std::function<void(bool)> ConnectionChangeHandler;
// class Websocket
// {
// private:
//     rtc::WebSocket *ws;
//     MessageProcessor processor;
//     std::atomic_bool connected;
//     std::atomic_bool running;
//     std::atomic_bool shouldStop;
//     SocketMessageHandler messageHandler;
//     ConnectionChangeHandler connectionChangeHandler;
//     std::string uri;
//          std::mutex _msgLck; 
   
//     void _init();
//     void _onOpen();
//     void _onClose();
//     void _onMessage(const std::string &msg);

// public:
//     Websocket(std::string url);
//     ~Websocket();
//     void setURL(std::string url);
//     bool isPieSocket();
//     void start();
//     void stop();
//     bool sendData(std::string recepient, MessageCode code, std::string data,bool compressed=false);
//     bool sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data,bool compressed=false);
//     bool isConnected();
//     void setConncted(bool connected);
//     void onMessage(SocketMessageHandler handler);
//     void onConnectionChange(ConnectionChangeHandler handler);
// };
// class WebsocketManager
// {
// private:
//     Websocket ws;
//      std::mutex _lck;
//     std::atomic_bool connected;
//     MessageProcessor processor;
//     void _handleMessage(Message msg);
//     void _connectTask();
//     std::atomic_bool running;
//     SocketMessageHandler messageHandler;
//     ConnectionChangeHandler connectionChangeHandler;

// public:
//     void setup();
//     WebsocketManager();
//     bool sendData(std::string recepient, MessageCode code, std::string data,bool compressed=false);
//     bool sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data,bool compressed=false);
//     ~WebsocketManager();
// };
