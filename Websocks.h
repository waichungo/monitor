#pragma once
#include <winsock2.h>
#include "Models.h"
#include "Utils.h"
#include "MessageProcessor.h"
#include "atomic"
#include "WebSocketClient.h"
// typedef NTSTATUS(__stdcall *NtQueryInformationProcessFunc)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
typedef void (*SocketMessageHandler)(Message msg);
typedef void (*ConnectionChangeHandler)(bool connected);
class Websocket
{
private:
    hv::WebSocketClient  ws;
    MessageProcessor processor;
    bool connected;
    bool running;
    bool shouldStop;
    SocketMessageHandler messageHandler;
    ConnectionChangeHandler connectionChangeHandler;
    std::string uri;
    void _onOpen();
    void _onClose();    
    void _onMessage(const std::string& msg);
    

public:
    Websocket(std::string url);
    void setURL(std::string url);
    bool isPieSocket();
    void start();
    void stop();
    void sendData(std::string recepient, MessageCode code, std::string data);
    bool isConnected();
    void onMessage(SocketMessageHandler handler);
    void onConnectionChange(ConnectionChangeHandler handler);
};
class WebsocketManager
{
private:
    Websocket ws;
    bool connected;
    MessageProcessor processor;
    void _handleMessage(Message msg);
    bool running;
public:
    void setup();
    WebsocketManager();
    void sendData(std::string recepient, MessageCode code, std::string data);
    ~WebsocketManager();
};
