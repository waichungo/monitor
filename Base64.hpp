#ifndef __BASE64_ENCODE_H
#define __BASE64_ENCODE_H
#include <cstdint>
#include <vector>
#include <string>

#include <cstring>


namespace base64
{
    const char base64_url_alphabet[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '-', '_'};

    /****************************************************************************
     * Public Types
     ****************************************************************************/

    /* Test structure content */

    typedef struct
    {
        const char *encoded_data;
        const char *plain_data;
    } base64_encode_test_t;

    class Base64
    {
    public:
        static std::string Encode(std::vector<uint8_t> &buffer)
        {
            std::string result = base64_encode(&buffer[0], buffer.size());
            return result;
        }
        static std::string Encode(std::string &text)
        {
            std::string result = base64_encode(reinterpret_cast<uint8_t *>(&text[0]), text.size());
            return result;
        }
        static std::vector<uint8_t> Decode(const char *data)
        {
            return base64_decode(data);
        }

    private:
        static std::string base64_encode(uint8_t *buffer, size_t len)
        {
            std::string out;
            if (len == 0)
            {
                return "";
            }
            out.reserve((len / 3) * 4);
            int val = 0, valb = -6;

            unsigned int i = 0;
            for (i = 0; i < len; i++)
            {
                unsigned char c = buffer[i];
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0)
                {
                    out.push_back(base64_url_alphabet[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }
            if (valb > -6)
            {
                out.push_back(base64_url_alphabet[((val << 8) >> (valb + 8)) & 0x3F]);
            }
            return out;
        }
        static std::vector<uint8_t> base64_decode(const char *buffer)
        {
            size_t len = std::strlen(buffer);
            std::vector<uint8_t> out;
            out.reserve(((len / 4) * 3) + 1);
            std::vector<int> T(256, -1);
            unsigned int i;
            for (i = 0; i < 64; i++)
                T[base64_url_alphabet[i]] = i;

            int val = 0, valb = -8;
            for (i = 0; i < len; i++)
            {
                unsigned char c = buffer[i];
                if (T[c] == -1)
                    break;
                val = (val << 6) + T[c];
                valb += 6;
                if (valb >= 0)
                {
                    out.push_back(char((val >> valb) & 0xFF));
                    valb -= 8;
                }
            }
            return out;
        }
    };

}

#endif