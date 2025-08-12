#include "eth_utils.h"
#include "rlp_encoder.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;
using boost::multiprecision::int128_t;
using std::vector;
using std::string;

//-------------------------------------
// Hex <-> Bytes helpers
//-------------------------------------
vector<uint8_t> hexToBytes(const std::string &hex) {
    std::string clean = hex;
    if (clean.rfind("0x", 0) == 0) clean = clean.substr(2);
    std::cout << "[hexToBytes] clean hex string length: " << clean.size() << "\n";
    if (clean.size() % 2 != 0) {
        throw std::runtime_error("Invalid hex string length: " + std::to_string(clean.size()));
    }

    std::vector<uint8_t> out(clean.size() / 2);
    for (size_t i = 0; i < clean.size(); i += 2) {
        out[i / 2] = (uint8_t) strtol(clean.substr(i, 2).c_str(), nullptr, 16);
    }
    return out;
}


// string bytesToHex(const vector<uint8_t> &data) {
//     std::ostringstream oss;
//     oss << std::hex << std::setfill('0');
//     for (auto b : data) {
//         oss << std::setw(2) << (int)b;
//     }
//     return oss.str();
// }

// string bytesToHex(const unsigned char *data, size_t len) {
//     std::ostringstream oss;
//     oss << std::hex << std::setfill('0');
//     for (size_t i = 0; i < len; ++i) {
//         oss << std::setw(2) << (int)data[i];
//     }
//     return oss.str();
// }

//-------------------------------------
// RLP Encoding
//-------------------------------------
// vector<uint8_t> rlp_encode_string(const vector<uint8_t> &input) {
//     if (input.size() == 1 && input[0] < 0x80) {
//         // Single byte, no prefix
//         return input;
//     } else if (input.size() <= 55) {
//         vector<uint8_t> out;
//         out.push_back(0x80 + input.size());
//         out.insert(out.end(), input.begin(), input.end());
//         return out;
//     } else {
//         throw std::runtime_error("RLP string too long for minimal impl");
//     }
// }

// vector<uint8_t> rlp_encode_integer(const cpp_int &value) {
//     if (value == 0) return {0x80}; // empty string per RLP spec
//     cpp_int tmp = value;
//     vector<uint8_t> bytes;
//     while (tmp > 0) {
//         bytes.insert(bytes.begin(), static_cast<uint8_t>(tmp & 0xFF));
//         tmp >>= 8;
//     }
//     return rlp_encode_string(bytes);
// }

// vector<uint8_t> rlp_encode_list(const vector<vector<uint8_t>> &items) {
//     vector<uint8_t> payload;
//     for (auto &i : items) {
//         payload.insert(payload.end(), i.begin(), i.end());
//     }
//     if (payload.size() <= 55) {
//         vector<uint8_t> out;
//         out.push_back(0xc0 + payload.size());
//         out.insert(out.end(), payload.begin(), payload.end());
//         return out;
//     } else {
//         throw std::runtime_error("RLP list too long for minimal impl");
//     }
// }

//-------------------------------------
// Keccak256 Hash
//-------------------------------------
// std::vector<uint8_t> keccak256(const std::vector<uint8_t>& input);
// vector<uint8_t> keccak256(const vector<uint8_t> &input) {
//     EVP_MD_CTX *ctx = EVP_MD_CTX_new();
//     const EVP_MD *md = EVP_get_digestbyname("keccak256");
//     if (!md) throw std::runtime_error("Keccak256 not available in OpenSSL");

//     unsigned char hash[EVP_MAX_MD_SIZE];
//     unsigned int len;
//     EVP_DigestInit_ex(ctx, md, NULL);
//     EVP_DigestUpdate(ctx, input.data(), input.size());
//     EVP_DigestFinal_ex(ctx, hash, &len);
//     EVP_MD_CTX_free(ctx);

//     return vector<uint8_t>(hash, hash + len);
// }

//-------------------------------------
// cpp_int to hex string
//-------------------------------------
string cpp_int_to_hex(const cpp_int& value) {
    std::stringstream ss;
    ss << std::hex << value;
    string hex_str = ss.str();
    if (hex_str[0] == '-') {
        throw std::invalid_argument("Negative values not supported for hex conversion.");
    }
    hex_str.erase(0, hex_str.find_first_not_of('0'));
    if (hex_str.empty()) hex_str = "0";
    return hex_str;
}

