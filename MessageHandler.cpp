#include "MessageHandler.h"
#include "protobufhelper/model.pb.h"
#include "Utils.h"
#include "MessageProcessor.h"
#include "App.h"
using json = nlohmann::json;
MessageHandler *defaultMessageHandler = nullptr;
std::string getDefaultWebsocketUri()
{
   std::string uri = OBFUSCATED("wss://commander-ws-server.onrender.com/?client=true&id=");
   // std::string uri = OBFUSCATED("ws://192.168.0.55:8000?client=true&id=");
   // std::string uri = OBFUSCATED("ws://localhost:8000?client=true&id=");
   uri += GetMachineID();

   return uri;
}
std::string getPieWebsocketUri()
{
   std::string uri = OBFUSCATED("wss://free.blr2.piesocket.com/v3/1?api_key=Yvd3PEoeTLU8BWcgPa4OXc2o2Ow1MnLRcjI2844A");
   return uri;
}
bool MessageHandler::sendData(std::string recepient, MessageCode code, std::vector<uint8_t> data, std::string meta, bool compressed)
{
   try
   {
      _msgLck.lock();
      std::string payload = VecToString(data);
      binmessage::BinaryMessage msg;
      msg.set_code(code);
      msg.set_compressed(compressed);
      msg.set_data(payload);
      msg.set_sender(GetMachineID());
      msg.set_recepient(recepient);
      msg.set_meta(meta);
      std::string encoded = msg.SerializeAsString();
      bool sent = ws.send((std::byte *)&encoded[0], encoded.size());
      _msgLck.unlock();
      return sent;
   }
   catch (const std::exception &e)
   {
      _msgLck.unlock();
      std::cerr << "Websocket send error:\n"
                << e.what() << '\n';
   }
   return false;
}
bool MessageHandler::sendData(std::string recepient, MessageCode code, std::string data, bool compressed)
{
   try
   {
      _msgLck.lock();
      json msg;
      msg["recepient"] = recepient;
      msg["code"] = (int)code;
      msg["sender"] = GetMachineID();
      msg["data"] = data;
      msg["compressed"] = compressed;
      bool sent = ws.send(msg.dump());
      _msgLck.unlock();
      return sent;
   }
   catch (const std::exception &e)
   {
      _msgLck.unlock();
      std::cerr << "Websocket send error:\n"
                << e.what() << '\n';
   }
   return false;
}

MessageHandler::MessageHandler()
{
   ws.onClosed((std::function<void()>)[&] {
      if (connected)
      {
         connected = false;
         std::cout << "Websocket closed\n";
      }
   });
   ws.onOpen((std::function<void()>)[&] {
      if (!connected)
      {
         std::cout << "Websocket opened\n";
         connected = true;
      }
   });
   messageHandler = (SocketMessageHandler)[&](Message message)
   {
      MessageProcessor::processMessage(message);
   };
   ws.onMessage([&](rtc::message_variant data)
                {
        if (messageHandler!=nullptr&& std::holds_alternative<rtc::string>(data)) {
        _onMessage(std::get<rtc::string>(data));
    } });
   ws.open(getDefaultWebsocketUri());
   std::thread th([&]()
                  { _connectTask(); });
   th.detach();
}
void MessageHandler::_connectTask()
{
   bool starting = true;
   while (true)
   {
      if (!this->connected && starting)
      {
         for (size_t i = 0; i < 100; i++)
         {
            if (this->connected)
            {
               break;
            }
            Sleep(200);
         }
         starting = false;
      }
      if (!this->connected)
      {
         if (!InternetIsWorking())
         {
            WaitForConnection();
         }
         Sleep(1000);
         if (!this->ws.isOpen())
         {
            this->ws.close();
            this->ws.open(getDefaultWebsocketUri());
         }
      }
      Sleep(2000);
   }
}
void MessageHandler::_onMessage(const std::string &msg)
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