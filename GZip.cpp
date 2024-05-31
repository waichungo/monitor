#include "GZip.h"
#include <iostream>
#include <vector>
#include <string>

#include <zlib.h>
#include <vector>
#include <string>

std::vector<uint8_t> compressGzip(void *input, size_t size)
{
    std::vector<uint8_t> compressed(size + 12); // A rough estimate of the compressed size
    try
    {
        z_stream strm = {};
        if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            throw std::runtime_error("Failed to initialize compression");
        }

        strm.next_in = static_cast<Bytef *>(input);
        strm.avail_in = size;
        strm.next_out = compressed.data();
        strm.avail_out = compressed.size();

        while (strm.avail_in > 0)
        {
            if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                throw std::runtime_error("Compression failed");
            }
        }

        compressed.resize(strm.total_out);
        deflateEnd(&strm);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return compressed;
}
std::vector<uint8_t> decompressGzip(void *input, size_t size)
{
    std::vector<uint8_t> decompressed(size * 3); // A rough estimate of the decompressed size

    try
    {
        z_stream strm = {};
        if (inflateInit2(&strm, MAX_WBITS + 16) != Z_OK)
        {
            throw std::runtime_error("Failed to initialize decompression");
        }

        strm.next_in = static_cast<Bytef *>(input);
        strm.avail_in = size;
        strm.next_out = decompressed.data();
        strm.avail_out = decompressed.size();

        while (strm.avail_in > 0)
        {
            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                inflateEnd(&strm);
                throw std::runtime_error("Decompression failed");
            }
            if (ret == Z_STREAM_END)
            {
                break;
            }

            if (strm.avail_out == 0)
            {
                decompressed.resize(decompressed.size() * 2);
                strm.next_out = decompressed.data() + strm.total_out;
                strm.avail_out = decompressed.size() - strm.total_out;
            }
        }
        decompressed.resize(strm.total_out);
        inflateEnd(&strm);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return decompressed;
}
std::vector<uint8_t> compressGzip(const std::vector<uint8_t> &input)
{
    return compressGzip((void *)&input[0], input.size());
}
std::vector<uint8_t> compressGzip(const std::string &input)
{
    return compressGzip((void *)&input[0], input.size());
}
std::vector<uint8_t> decompressGzip(const std::vector<uint8_t> &input)
{
    return decompressGzip((void *)&input[0], input.size());
}
// std::vector<ui nt8_t> compressGzip(const std::vector<uint8_t> &input)
// {
//     std::vector<uint8_t> compressed(input.size() + 12); // A rough estimate of the compressed size
//     try
//     {
//         z_stream strm = {};
//         if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
//         {
//             throw std::runtime_error("Failed to initialize compression");
//         }

//         strm.next_in = const_cast<Bytef *>(input.data());
//         strm.avail_in = input.size();
//         strm.next_out = compressed.data();
//         strm.avail_out = compressed.size();

//         while (strm.avail_in > 0)
//         {
//             if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR)
//             {
//                 deflateEnd(&strm);
//                 throw std::runtime_error("Compression failed");
//             }
//         }

//         compressed.resize(strm.total_out);
//         deflateEnd(&strm);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     return compressed;
// }
