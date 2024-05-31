#include "Encrypter.h"

#include "MetaString.h"
#include "AES.h"

using namespace andrivet::ADVobfuscator;
std::string VecToString(std::vector<unsigned char> &vec);
std::vector<unsigned char> StringToVec(std::string &stringdata);
std::vector<uint8_t> _iv;
std::vector<uint8_t> GetIV()
{
    if (_iv.size() > 0)
        return _iv;
    auto IV = OBFUSCATED("78yutojfnp[rhdyu");
    std::vector<uint8_t> result(16);
    memcpy(&result.data()[0], IV, 16);
    _iv = result;
    return result;
}

std::string Encrypter::Encrypt(std::string data, std::string pass, std::vector<uint8_t> iv)
{
    uint64_t origSize = data.size();
    std::string result;

    auto plainBuffer = StringToVec(data);
    data = "";

    auto passBuffer = StringToVec(pass);

    auto rem = passBuffer.size() % 16;
    auto pad = 16 - rem;

    if (rem > 0)
    {

        passBuffer.resize(passBuffer.size() + pad);
    }

    rem = plainBuffer.size() % 16;
    pad = 16 - rem;

    if (rem > 0)
    {
        plainBuffer.resize(plainBuffer.size() + pad);
    }

    plainBuffer.resize(plainBuffer.size() + 16);
    uint64_t *ref = (uint64_t *)&plainBuffer.data()[plainBuffer.size() - 8];
    *ref = origSize;

    AES aes(AESKeyLength::AES_128);

    auto encrypted = aes.EncryptCBC(plainBuffer, passBuffer, iv);

    // std::cout << "Encrypted buffer to string\n";
    auto EncPlacer = VecToString(encrypted);

    // std::cout << "Base encoding encrypted\n";
    result = base64::Base64::Encode(EncPlacer);

    return result;
}
std::string Encrypter::Decrypt(std::string data, std::string pass, std::vector<uint8_t> iv)
{
    std::string result;
    string out = "";

    auto plainBuffer = base64::Base64::Decode(data.c_str());
    auto passBuffer = StringToVec(pass);

    auto rem = passBuffer.size() % 16;
    auto pad = 16 - rem;

    if (rem > 0)
    {
        passBuffer.resize(passBuffer.size() + pad);
    }
    AES aes(AESKeyLength::AES_128);

    auto decrypted = aes.DecryptCBC(plainBuffer, passBuffer, iv);
    uint64_t *ref = (uint64_t *)&decrypted.data()[decrypted.size() - 8];
    if (*ref < decrypted.size())
    {
        decrypted.resize(*ref);
    }

    result.resize(decrypted.size());

    memcpy(&result[0], &decrypted[0], decrypted.size());
    return result;
}

vector<uint8_t> Encrypter::EncryptData(std::vector<uint8_t> &data, std::string pass, std::vector<uint8_t> iv)
{

    auto passBuffer = StringToVec(pass);

    auto rem = passBuffer.size() % 16;
    uint64_t size = data.size();

    if (rem > 0)
    {
        auto pad = 16 - rem;
        passBuffer.resize(passBuffer.size() + pad);
    }
    rem = data.size() % 16;

    if (rem > 0)
    {
        auto pad = 16 - rem;
        data.resize(data.size() + pad);
    }
    auto size64 = sizeof(size);
    data.resize(data.size() + size64 * 2);

    uint64_t *ref = (uint64_t *)&data.data()[data.size() - (sizeof(size))];
    *ref = size;

    // result = aes.EncryptCBC(data, passBuffer, GetIV());
    AES aes(AESKeyLength::AES_128);

    auto encrypted = aes.EncryptCBC(data, passBuffer, iv);

    return encrypted;
}
vector<uint8_t> Encrypter::DecryptData(vector<uint8_t> &data, std::string pass, std::vector<uint8_t> iv)
{
    // vector<uint8_t> result;
    if (data.size() > 0)
    {
        auto passBuffer = StringToVec(pass);
        auto rem = passBuffer.size() % 16;

        if (rem > 0)
        {
            auto pad = 16 - rem;
            passBuffer.resize(passBuffer.size() + pad);
        }

        // result = aes.DecryptCBC(data, passBuffer, GetIV());

        AES aes(AESKeyLength::AES_128);

        auto decrypted = aes.DecryptCBC(data, passBuffer, iv);
        // void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, size_t length);

        uint64_t *size = (uint64_t *)&decrypted.data()[decrypted.size() - ((sizeof(uint64_t)))];

        if (*size < decrypted.size())
        {
            decrypted.resize(*size);
            return decrypted;
        }
    }
    return std::vector<uint8_t>();
}

std::string GetPassword()
{
    string pass = OBFUSCATED("?>7*%e9on&$%$9");
    return pass;
}
