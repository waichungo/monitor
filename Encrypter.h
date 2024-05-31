#pragma once

#include <regex>
#include "Base64.hpp"
#include "vector"
#include "stdint.h"
#include "stringutils.h"
using std::string;
using std::vector;

std::string GetPassword();
std::vector<uint8_t> GetIV();

class Encrypter
{
    
public:
    static std::string Decrypt(std::string data,std::string pass=GetPassword(),std::vector<uint8_t> iv=GetIV());
    static std::string Encrypt(std::string data,std::string pass=GetPassword(),std::vector<uint8_t> iv=GetIV());
    static vector<uint8_t> DecryptData(vector<uint8_t> &data,std::string pass=GetPassword(),std::vector<uint8_t> iv=GetIV());
    static vector<uint8_t> EncryptData(vector<uint8_t> &data,std::string pass=GetPassword(),std::vector<uint8_t> iv=GetIV());
};