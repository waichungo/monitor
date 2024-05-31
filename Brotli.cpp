#include <brotli/encode.h>
#include <brotli/decode.h>

#include <string>
#include <vector>
#include <array>

#ifndef BROTLI_BUFFER_SIZE
#define BROTLI_BUFFER_SIZE 1024
#endif

namespace brotli
{
     std::vector<uint8_t> compress(void *data, size_t size)
    {
        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer;
        std::vector<uint8_t> result;
        result.reserve(size);

        size_t available_in = size, available_out = buffer.size();
        const uint8_t *next_in = reinterpret_cast<const uint8_t *>(data);
        uint8_t *next_out = buffer.data();

        do
        {
            BrotliEncoderCompressStream(
                instance, BROTLI_OPERATION_FINISH,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            int payloadSize = buffer.size() - available_out;
            int pos = result.size();

            result.resize(result.size() + payloadSize);
            memcpy(&result[pos], buffer.data(), payloadSize);
            available_out = buffer.size();
            next_out = buffer.data();
        } while (!(available_in == 0 && BrotliEncoderIsFinished(instance)));

        BrotliEncoderDestroyInstance(instance);
        return result;
    }

     std::vector<uint8_t> decompress(void *data, size_t size)
    {
        auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer;
        std::vector<uint8_t> result;
        result.reserve((size * 2) + int(size / 2));

        size_t available_in = size, available_out = buffer.size();
        const uint8_t *next_in = reinterpret_cast<const uint8_t *>(data);
        uint8_t *next_out = buffer.data();
        BrotliDecoderResult oneshot_result;

        do
        {
            oneshot_result = BrotliDecoderDecompressStream(
                instance,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            int payloadSize = buffer.size() - available_out;

            int pos = result.size();

            result.resize(result.size() + payloadSize);
            memcpy(&result[pos], buffer.data(), payloadSize);
            available_out = buffer.size();
            next_out = buffer.data();
        } while (!(available_in == 0 && oneshot_result == BROTLI_DECODER_RESULT_SUCCESS));

        BrotliDecoderDestroyInstance(instance);
        return result;
    }
    std::vector<uint8_t> compress(const std::vector<uint8_t> &data)
    {
        return compress((void*)&data[0], data.size());
    }
    std::vector<uint8_t> compress(const std::string &data)
    {
        return compress((void*)&data[0], data.size());
    }
    std::vector<uint8_t> decompress(const std::vector<uint8_t> &data)
    {
        return decompress((void*)&data[0], data.size());
    }
    
}