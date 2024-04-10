#include "Websocks.h"
#include "rtc/rtc.hpp"
#include "Utils.h"
#include "Models.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <string>

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
void RunSocks()
{
    std::string uri = "ws://localhost:8000?id=";
    rtc::WebSocket ws;
    bool running = false;
    bool disconnected = false;
    ws.onClosed([&]()
                {
                    running = false;
                    disconnected=true;
                     std::cout << "WebSocket closed" << std::endl; });
    ws.onOpen([&]()
              {
                running=true;
                std::cout << "WebSocket open" << std::endl; });
    ws.onError([&](std::string err)
               { std::cout << "WebSocket error: " << err << std::endl; });

    ws.onMessage([](std::variant<rtc::binary, rtc::string> message)
                 {
    if (std::holds_alternative<rtc::string>(message)) {
       auto msg= std::get<rtc::string>(message) ;
       json jsn;
       try
       {
        jsn=json::parse(msg);
        if(jsn.contains("recepient")&&jsn["recepient"].get<std::string>()==GetMachineID()){
            std::cout<<"Got message: "<<msg<<"\n";
            Message parsed;
            parsed.code=jsn.contains("code")?((MessageCode) jsn["code"].get<int>()):MC_UNDEFINED;
            parsed.data=jsn.contains("data")? jsn["data"].get<std::string>():"";
            parsed.recepient=jsn.contains("recepient")? jsn["recepient"].get<std::string>():"";
            parsed.sender=jsn.contains("sender")? jsn["sender"].get<std::string>():"";

            std::cout<<"";
        }
       }
       catch(const std::exception& e)
       {
        std::cerr << e.what() << '\n';
       }
       
    } });

    // std::string uri = "ws://localhost:8000?id=";
    uri += GetMachineID();
    // ws.open("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A&notify_self=1");
    ws.open(uri);
    std::thread th([&]()
                   {
                       while (true)
                       {
                           Sleep(500);
                           if (disconnected)
                           {
                               disconnected = false;
                               ws.open(uri);
                           }
                           else if (running)
                           {
                               json msg;
                               msg["recepient"] = "HUHUGEEG";
                               msg["code"] = 8;
                               msg["sender"] = GetMachineID();
                               msg["data"] = "";
                               ws.send(msg.dump());
                           }
                       } });
    th.join();
}