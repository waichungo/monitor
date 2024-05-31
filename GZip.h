#pragma once
#include<vector>
#include<string>
std::vector<uint8_t> compressGzip(void *input, size_t size);
std::vector<uint8_t> decompressGzip(void *input, size_t size);
std::vector<uint8_t> compressGzip(const std::vector<uint8_t> &input);
std::vector<uint8_t> compressGzip(const std::string &input);
std::vector<uint8_t> decompressGzip(const std::vector<uint8_t> &input);