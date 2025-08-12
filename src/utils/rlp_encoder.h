#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>
using boost::multiprecision::cpp_int;

using Byte = uint8_t;
using Bytes = std::vector<Byte>;

// Helper: encode length as big endian
inline Bytes encodeLength(size_t len, Byte offset) {
    if (len <= 55) {
        return Bytes{static_cast<Byte>(offset + len)};
    } else {
        // length of len in bytes
        Bytes lenBytes;
        size_t tmp = len;
        while (tmp > 0) {
            lenBytes.insert(lenBytes.begin(), static_cast<Byte>(tmp & 0xFF));
            tmp >>= 8;
        }
        if (lenBytes.size() > 8)
            throw std::runtime_error("Input too long");

        Bytes result;
        result.push_back(static_cast<Byte>(offset + 55 + lenBytes.size()));
        result.insert(result.end(), lenBytes.begin(), lenBytes.end());
        return result;
    }
}

// RLP encode string or byte array
inline Bytes rlp_encode_string(const Bytes &input) {
    if (input.size() == 1 && input[0] < 0x80) {
        // Single byte, value < 0x80: encoded as is
        return input;
    }
    Bytes prefix = encodeLength(input.size(), 0x80);
    Bytes out = prefix;
    out.insert(out.end(), input.begin(), input.end());
    return out;
}

// Overload: encode string (hex) input
inline Bytes rlp_encode_string(const std::string &input) {
    return rlp_encode_string(Bytes(input.begin(), input.end()));
}

// RLP encode integer as minimal big endian bytes, then as string
inline Bytes rlp_encode_integer(const boost::multiprecision::cpp_int &val) {
    if (val == 0) return Bytes{0x80};  // RLP encoding of empty string means zero

    boost::multiprecision::cpp_int tmp = val;
    Bytes buf;
    while (tmp > 0) {
        buf.insert(buf.begin(), static_cast<Byte>((tmp & 0xFF).convert_to<unsigned int>()));
        tmp >>= 8;
    }
    return rlp_encode_string(buf);
}

// RLP encode list of byte arrays
inline Bytes rlp_encode_list(const std::vector<Bytes> &items) {
    Bytes payload;
    for (const auto &item : items) {
        payload.insert(payload.end(), item.begin(), item.end());
    }
    Bytes prefix = encodeLength(payload.size(), 0xc0);
    Bytes out = prefix;
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

// Utility to convert Bytes to hex string with 0x prefix
inline std::string bytesToHex(const Byte* data, size_t size) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string hex = "0x";
    for (size_t i = 0; i < size; ++i) {
        hex += hex_chars[(data[i] >> 4) & 0x0F];
        hex += hex_chars[data[i] & 0x0F];
    }
    return hex;
}
