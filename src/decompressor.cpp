#include "decompressor.h"

std::string decompressor::decompress(const std::string& compressed_data, const std::string& l_encoding) {
    std::string encoding = normalize_encoding(l_encoding);

    if (encoding == "identity" || encoding.empty()) {
        return compressed_data;
    }
    else if (encoding == "gzip") {
        return decompress_gzip(compressed_data);
    }
    else if (encoding == "deflate") {
        return decompress_deflate(compressed_data);
    }
    else if (encoding == "brotli") {
        return decompress_brotli(compressed_data);
    }
    else if (encoding == "zstd") {
        return decompress_zstd(compressed_data);
    }
    else {
        throw std::runtime_error("Unsupported encoding: " + encoding);
    }
}

std::string decompressor::normalize_encoding(const std::string& content_encoding) {
    std::string encoding;
    for (char c : content_encoding) {
        encoding += tolower(c);
    }

    if (encoding.find("gzip") != std::string::npos) return "gzip";
    if (encoding.find("deflate") != std::string::npos) return "deflate";
    if (encoding.find("br") != std::string::npos) return "brotli";
    if (encoding.find("zstd") != std::string::npos) return "zstd";

    return encoding;
}

std::string decompressor::decompress_gzip(const std::string& compressed_data) {
    return decompress_zlib(compressed_data, 15 + 16);
}

std::string decompressor::decompress_deflate(const std::string& compressed_data) {
    return decompress_zlib(compressed_data, -15);
}

std::string decompressor::decompress_zlib(const std::string& compressed_data, int window_bits) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, window_bits) != Z_OK) {
        throw std::runtime_error("inflateInit2 failed");
    }

    zs.next_in = (Bytef*)compressed_data.data();
    zs.avail_in = compressed_data.size();

    const size_t BUFFER_SIZE = 32768;
    std::vector<char> outbuffer(BUFFER_SIZE);

    int ret = Z_OK;
    std::string result;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer.data());
        zs.avail_out = outbuffer.size();

        ret = inflate(&zs, Z_NO_FLUSH);

        if (ret == Z_OK || ret == Z_STREAM_END) {
            result.append(outbuffer.data(), outbuffer.size() - zs.avail_out);
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("zlib decompression failed: " + std::to_string(ret));
    }

    return result;
}

std::string decompressor::decompress_brotli(const std::string& compressed_data) {
    size_t available_in = compressed_data.size();
    const uint8_t* next_in = (const uint8_t*)compressed_data.data();

    size_t available_out = compressed_data.size() * 5;
    std::vector<uint8_t> result_buffer(available_out);
    size_t total_out = 0;

    BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!state) {
        throw std::runtime_error("Failed to create Brotli decoder");
    }

    BrotliDecoderResult result;
    do {
        size_t remaining_out = result_buffer.size() - total_out;
        uint8_t* next_out = result_buffer.data() + total_out;

        result = BrotliDecoderDecompressStream(
            state, &available_in, &next_in, &remaining_out, &next_out, nullptr);

        total_out = next_out - result_buffer.data();

        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            result_buffer.resize(result_buffer.size() * 2);
        }
    } while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT);

    BrotliDecoderDestroyInstance(state);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
        throw std::runtime_error("Brotli decompression failed");
    }

    return std::string(result_buffer.begin(), result_buffer.begin() + total_out);
}

std::string decompressor::decompress_zstd(const std::string& compressed_data) {
    unsigned long long const decompressed_size = ZSTD_getFrameContentSize(
        compressed_data.data(), compressed_data.size());

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
        throw std::runtime_error("ZSTD: Not a valid compressed frame");
    }

    if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        return decompress_zstd_streaming(compressed_data);
    }

    std::string result;
    result.resize(decompressed_size);

    size_t const actual_decompressed_size = ZSTD_decompress(
        (void*)result.data(), result.size(),
        compressed_data.data(), compressed_data.size()
    );

    if (ZSTD_isError(actual_decompressed_size)) {
        throw std::runtime_error("ZSTD decompression failed: " +
            std::string(ZSTD_getErrorName(actual_decompressed_size)));
    }

    if (actual_decompressed_size != decompressed_size) {
        result.resize(actual_decompressed_size);
    }

    return result;
}

std::string decompressor::decompress_zstd_streaming(const std::string& compressed_data) {
    ZSTD_DCtx* const dctx = ZSTD_createDCtx();
    if (!dctx) {
        throw std::runtime_error("ZSTD: Failed to create decompression context");
    }

    const size_t out_buffer_size = ZSTD_DStreamOutSize();
    std::vector<char> out_buffer(out_buffer_size);
    std::string result;

    ZSTD_inBuffer input = { compressed_data.data(), compressed_data.size(), 0 };

    while (input.pos < input.size) {
        ZSTD_outBuffer output = { out_buffer.data(), out_buffer_size, 0 };

        size_t const ret = ZSTD_decompressStream(dctx, &output, &input);

        if (ZSTD_isError(ret)) {
            ZSTD_freeDCtx(dctx);
            throw std::runtime_error("ZSTD streaming decompression failed: " +
                std::string(ZSTD_getErrorName(ret)));
        }

        result.append(out_buffer.data(), output.pos);
    }

    ZSTD_freeDCtx(dctx);
    return result;
}