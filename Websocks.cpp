// #include <winsock2.h>
// #include "Websocks.h"

// #include <nlohmann/json.hpp>
// #include <iostream>
// #include <string>
// #include <mutex>
// #include <memory>
// #include <functional>

// #include "MessageProcessor.h"

// using json = nlohmann::json;
// // std::shared_ptr<::Websocket> getDefaultWebsocket()
// // {
// //     std::string uri = OBFUSCATED("ws://localhost:8000?id=");
// //     uri += GetMachineID();
// //     auto ws = std::make_shared<::Websocket>(::Websocket(uri));

// //     return ws;
// // }
// // std::shared_ptr<::Websocket> getPieWebsocket()
// // {
// //     std::string uri = OBFUSCATED("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A");
// //     auto ws = std::make_shared<::Websocket>(::Websocket(uri));

// //     return ws;
// // }
// std::string getDefaultWebsocketUri()
// {
//     std::string uri = OBFUSCATED("ws://192.168.0.20:8000?client=true&id=");
//     // std::string uri = OBFUSCATED("ws://localhost:8000?client=true&id=");
//     uri += GetMachineID();

//     return uri;
// }
// std::string getPieWebsocketUri()
// {
//     std::string uri = OBFUSCATED("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A");
//     return uri;
// }
// WebsocketManager::WebsocketManager() : _lck(std::mutex()), ws(""), processor(MessageProcessor())
// {
//     ws.setURL(getDefaultWebsocketUri());
// }
// void WebsocketManager::_handleMessage(Message msg)
// {
//     processor.processMessage(msg);
// }

// void Websocket::setConncted(bool connected)
// {
//     this->connected = connected;
// }
// void WebsocketManager::_connectTask()
// {
//     if (_lck.try_lock())
//     {
//         bool starting = true;
//         Sleep(1000);
//         while (true)
//         {
//             if (!this->connected && starting)
//             {
//                 for (size_t i = 0; i < 100; i++)
//                 {
//                     if (this->connected)
//                     {
//                         break;
//                     }
//                     Sleep(200);
//                 }

//                 starting = false;
//             }
//             if (!this->connected)
//             {

//                 if (!InternetIsWorking())
//                 {
//                     WaitForConnnection();
//                 }
//                 Sleep(1000);
//                 if (!this->ws.isConnected())
//                 {
//                     this->ws.stop();
//                     this->ws.setURL(getDefaultWebsocketUri());
//                     // this->ws.setURL(this->ws.isPieSocket() ? getDefaultWebsocketUri() : getPieWebsocketUri());
//                     this->ws.start();
//                 }
//             }

//             Sleep(2000);
//         }
//     }
// }
// void WebsocketManager::setup()
// {

//     connectionChangeHandler = [&](bool opened)
//     {
//         this->connected = opened;
//     };

//     messageHandler = [&](Message ms)
//     {
//         this->_handleMessage(ms);
//     };

//     this->ws.onMessage(messageHandler);
//     this->ws.onConnectionChange(connectionChangeHandler);
//     this->ws.start();
//     Sleep(1000);
//     std::thread th([&]()
//                    { this->_connectTask(); });
//     th.detach();
// }
// WebsocketManager::~WebsocketManager()
// {
// }
// void Websocket::onMessage(SocketMessageHandler handler)
// {
//     if (handler != nullptr)
//     {
//         messageHandler = handler;
//     }
// }

// bool Websocket::isPieSocket()
// {
//     return StringUtils::contains(uri, OBFUSCATED("piesocket.com"));
// }
// bool WebsocketManager::sendData(std::string recepient, MessageCode code, std::string data, bool compressed)
// {

//     if (ws.isConnected())
//     {
//         return ws.sendData(recepient, code, data, compressed);
//     }
//     return false;
// }
// bool WebsocketManager::sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data, bool compressed)
// {

//     if (ws.isConnected())
//     {
//         return ws.sendData(recepient, code, data, compressed);
//     }
//     return false;
// }
// bool Websocket::sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data, bool compressed)
// {
//     try
//     {
//         _msgLck.lock();
//         std::string payload=VecToString(data);
//         binmessage::BinaryMessage msg;
//         msg.set_code(code);
//         msg.set_compressed(compressed);
//         msg.set_data(payload);
//         msg.set_sender(GetMachineID());
//         msg.set_recepient(recepient);
//         std::string encoded = msg.SerializeAsString();
//         bool res = ws->send((std::byte*)&encoded[0],encoded.size());
//         _msgLck.unlock();
//         return res;
//     }
//     catch (const std::exception &e)
//     {
//         _msgLck.unlock();
//         std::cerr << "Websocket send error:\n"
//                   << e.what() << '\n';
//     }
//     return false;
// }
// bool Websocket::sendData(std::string recepient, MessageCode code, std::string data, bool compressed)
// {
//     try
//     {
//         _msgLck.lock();
//         json msg;
//         msg["recepient"] = recepient;
//         msg["code"] = (int)code;
//         msg["sender"] = GetMachineID();
//         msg["data"] = data;
//         msg["compressed"] = compressed;
//         ws->send(msg.dump());
//         _msgLck.unlock();
//         return true;
//     }
//     catch (const std::exception &e)
//     {
//         _msgLck.unlock();
//         std::cerr << "Websocket send error:\n"
//                   << e.what() << '\n';
//     }
//     return false;
// }
// void Websocket::_onMessage(const std::string &msg)
// {
//     if (messageHandler != nullptr)
//     {
//         json jsn;
//         try
//         {
//             jsn = json::parse(msg);
//             if (jsn.contains("recepient") && jsn["recepient"].get<std::string>() == GetMachineID())
//             {
//                 std::cout << "Got message: " << msg << "\n";
//                 Message parsed;
//                 parsed.code = jsn.contains("code") ? ((MessageCode)jsn["code"].get<int>()) : MC_UNDEFINED;
//                 parsed.data = jsn.contains("data") ? jsn["data"].get<std::string>() : "";
//                 parsed.recepient = jsn.contains("recepient") ? jsn["recepient"].get<std::string>() : "";
//                 parsed.sender = jsn.contains("sender") ? jsn["sender"].get<std::string>() : "";
//                 messageHandler(parsed);
//             }
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << e.what() << '\n';
//         }
//     }
// }
// void Websocket::onConnectionChange(ConnectionChangeHandler handler)
// {
//     if (handler != nullptr)
//     {
//         connectionChangeHandler = handler;
//     }
// }
// void Websocket::_init()
// {

//     ws = new rtc::WebSocket();

//     ws->onClosed((std::function<void()>)std::bind(&Websocket::_onClose, this));
//     ws->onOpen((std::function<void()>)std::bind(&Websocket::_onOpen, this));
//     ws->onMessage([&](rtc::message_variant data)
//                   {
//         if (messageHandler!=nullptr&& std::holds_alternative<rtc::string>(data)) {
//         _onMessage(std::get<rtc::string>(data));
//     } });
// }
// Websocket::~Websocket()
// {
//     ws->close();
//     delete ws;
//     ws = nullptr;
// }
// Websocket::Websocket(std::string url) : _msgLck(std::mutex()), shouldStop(false), uri(url), messageHandler(nullptr), ws(nullptr), connected(false), running(false)
// {
//     _init();
//     // ws.onmessage = (std::function<void(const std::string& msg)> ) ((void*)std::bind(&Websocket::_onMessage, this));
// }
// void Websocket::stop()
// {
//     shouldStop = true;
//     Sleep(100);
//     if (running)
//     {
//         Sleep(550);
//     }
// }

// void Websocket::_onOpen()
// {

//     if (connectionChangeHandler != nullptr)
//     {
//         connectionChangeHandler(true);
//     }
//     std::cout << "WebSocket open" << std::endl;
// }

// void Websocket::_onClose()
// {
//     connected = false;
//     if (connectionChangeHandler != nullptr)
//     {
//         connectionChangeHandler(connected);
//     }
//     std::cout << "WebSocket closed" << std::endl;
// }
// void Websocket::setURL(std::string url)
// {
//     uri = url;
// }
// void Websocket::start()
// {
//     if (running)
//     {
//         for (size_t i = 0; i < 5; i++)
//         {
//             if (!running)
//             {
//                 break;
//             }
//             Sleep(2000);
//         }
//     }

//     if (running)
//     {
//         return;
//     }

//     std::thread th([&]()
//                    {
              
//                     this->shouldStop=false;
//                     this->running=true;
//                     // this->ws->url=this->uri;
//                     this->ws->open(this->uri);
//                     // this->ws->open(this->uri.c_str());
//                        while (!this->shouldStop)
//                        {               
//                          Sleep(1000);            
//                             if(!this->ws->isOpen()){                                             
//                                break;
//                            }else{
//                             if(!this->connected){
//                                 this->connected=true;
//                             }
//                            }
                           
//                        }
//                            Sleep(500);
                       
//                        this->running = false; });

//     th.detach();
// }
// bool Websocket::isConnected()
// {
//     return ws->isOpen();
// }