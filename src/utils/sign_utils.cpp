#include "sign_utils.h"
#include "eth_utils.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include <nlohmann/json.hpp>
#include <stdexcept>

using nlohmann::json;

std::string signTransactionImpl(const json &tx, const std::string &privKeyHex) {
    // Parse fields from JSON assuming string literals for numbers
    auto nonce       = boost::multiprecision::cpp_int(tx["nonce"].get<std::string>());
    auto gasPrice    = boost::multiprecision::cpp_int(tx["gasPrice"].get<std::string>());
    auto gasLimit    = boost::multiprecision::cpp_int(tx["gas"].get<std::string>());
    auto toBytes     = hexToBytes(tx["to"].get<std::string>());
    auto value       = boost::multiprecision::cpp_int(tx["value"].get<std::string>());
    auto dataBytes   = hexToBytes(tx["data"].get<std::string>());
    auto chainId     = boost::multiprecision::cpp_int(tx["chainId"].get<std::string>());
    auto privKeyBytes= hexToBytes(privKeyHex);

    // RLP encode unsigned transaction (v,r,s = chainId,0,0 per EIP-155)
    std::vector<std::vector<uint8_t>> fields = {
        rlp_encode_integer(nonce),
        rlp_encode_integer(gasPrice),
        rlp_encode_integer(gasLimit),
        rlp_encode_string(toBytes),
        rlp_encode_integer(value),
        rlp_encode_string(dataBytes),
        rlp_encode_integer(chainId),
        rlp_encode_integer(0),
        rlp_encode_integer(0)
    };
    auto unsignedTx = rlp_encode_list(fields);

    // Hash the RLP encoded unsigned transaction with keccak256
    auto hash = keccak256(unsignedTx);

    // Initialize secp256k1 context and sign
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_ecdsa_recoverable_signature sig;
    if (privKeyBytes.size() != 32) throw std::runtime_error("Private key must be 32 bytes");
    if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, hash.data(), privKeyBytes.data(), nullptr, nullptr)) {
        secp256k1_context_destroy(ctx);
        throw std::runtime_error("Signing failed");
    }

    unsigned char output64[64];
    int recid;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, output64, &recid, &sig);
    secp256k1_context_destroy(ctx);

    // Extract r, s values from signature
    std::vector<uint8_t> r(output64, output64 + 32);
    std::vector<uint8_t> s(output64 + 32, output64 + 64);

    // Calculate v value per EIP-155
    boost::multiprecision::cpp_int v = recid + 35 + (chainId * 2);

    // RLP encode final signed transaction including v, r, s
    std::vector<std::vector<uint8_t>> signedFields = {
        rlp_encode_integer(nonce),
        rlp_encode_integer(gasPrice),
        rlp_encode_integer(gasLimit),
        rlp_encode_string(toBytes),
        rlp_encode_integer(value),
        rlp_encode_string(dataBytes),
        rlp_encode_integer(v),
        rlp_encode_string(r),
        rlp_encode_string(s)
    };
    auto signedTx = rlp_encode_list(signedFields);

    // Return hex string with "0x" prefix
    return "0x" + bytesToHex(signedTx.data(), signedTx.size());
}
std::string signTransaction(
    const nlohmann::json& tx,
    const std::string& privateKey
) {
    return signTransactionImpl(tx, privateKey);
}
