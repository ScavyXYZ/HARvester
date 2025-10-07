#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <zlib.h>
#include <brotli/decode.h>
#include <zstd.h>

class decompressor {
public:
    static std::string decompress(const std::string& compressed_data, const std::string& l_encoding);

private:
    static std::string normalize_encoding(const std::string& content_encoding);

    static std::string decompress_gzip(const std::string& compressed_data);
    static std::string decompress_deflate(const std::string& compressed_data);
    static std::string decompress_zlib(const std::string& compressed_data, int window_bits);
    static std::string decompress_brotli(const std::string& compressed_data);
    static std::string decompress_zstd(const std::string& compressed_data);
    static std::string decompress_zstd_streaming(const std::string& compressed_data);
};