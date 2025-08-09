#ifndef SIGN_UTILS_H
#define SIGN_UTILS_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Hex string to bytes
std::vector<unsigned char> hexToBytes(const std::string &hex);

// Bytes to hex string (prefix with "0x")
std::string bytesToHex(const unsigned char *data, size_t len);

// Minimal RLP element encoding
std::vector<unsigned char> rlpEncodeElement(const std::vector<unsigned char> &input);

// Minimal RLP list encoding
std::vector<unsigned char> rlpEncodeList(const std::vector<std::vector<unsigned char>> &elements);

// Forward declaration of the real implementation
std::string signTransactionImpl(const nlohmann::json &tx, const std::string &privKeyHex);

// Declaration of the interface function (the one called externally)
std::string signTransaction(const nlohmann::json &tx, const std::string &privKeyHex);



#endif // SIGN_UTILS_H
