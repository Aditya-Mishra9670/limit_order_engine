#ifndef ETH_UTILS_H
#define ETH_UTILS_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "rlp_encoder.h"

// Short alias for json type
using json = nlohmann::json;

// --- Public API ---

/**
 * @brief Sign an Ethereum transaction and return the signed RLP-encoded transaction hex string.
 * 
 * @param tx JSON object containing transaction fields:
 *           {
 *              "nonce": "...",
 *              "gasPrice": "...",
 *              "gas": "...",
 *              "to": "0x...",
 *              "value": "...",
 *              "data": "0x..."
 *           }
 * 
 */

/**
 * @brief Compute Keccak-256 hash.
 * 
 * @param input Bytes to hash
 * @return std::vector<uint8_t> Keccak256 hash bytes
 */

// RLP encoding
// std::vector<uint8_t> rlp_encode_integer(const boost::multiprecision::cpp_int& value);
// std::vector<uint8_t> rlp_encode_string(const std::vector<uint8_t>& input);
// std::vector<uint8_t> rlp_encode_list(const std::vector<std::vector<uint8_t>>& items);
std::vector<uint8_t> keccak256(const std::vector<uint8_t> &input);
std::string cpp_int_to_hex(const boost::multiprecision::cpp_int& value);
#endif // ETH_UTILS_H
