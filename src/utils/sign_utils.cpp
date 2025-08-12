#include "sign_utils.h"
#include "utils/rlp_encoder.h"

#include "eth_utils.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include <nlohmann/json.hpp>
#include <stdexcept>

using nlohmann::json;
std::string signTransactionImpl(const json &tx, const std::string &privKeyHex)
{
    std::cout << "[signTransaction] Starting signing process...\n";

    std::cout << "[signTransaction] Accessing nonce...\n";
    if (!tx.contains("nonce"))
        throw std::runtime_error("Missing field: nonce");
    auto nonce = boost::multiprecision::cpp_int(tx["nonce"].get<std::string>());
    std::cout << "[signTransaction] nonce = " << nonce << "\n";

    std::cout << "[signTransaction] Accessing gasPrice...\n";
    if (!tx.contains("gasPrice"))
        throw std::runtime_error("Missing field: gasPrice");
    auto gasPrice = boost::multiprecision::cpp_int(tx["gasPrice"].get<std::string>());
    std::cout << "[signTransaction] gasPrice = " << gasPrice << "\n";

    std::cout << "[signTransaction] Accessing gas (gasLimit)...\n";
    if (!tx.contains("gas"))
        throw std::runtime_error("Missing field: gas");
    auto gasLimit = boost::multiprecision::cpp_int(tx["gas"].get<std::string>());
    std::cout << "[signTransaction] gasLimit = " << gasLimit << "\n";

    std::cout << "[signTransaction] Accessing to address...\n";
    if (!tx.contains("to"))
        throw std::runtime_error("Missing field: to");
    auto toBytes = hexToBytes(tx["to"].get<std::string>());
    std::cout << "[signTransaction] to address bytes size = " << toBytes.size() << "\n";

    std::cout << "[signTransaction] Accessing value...\n";
    if (!tx.contains("value"))
        throw std::runtime_error("Missing field: value");
    auto value = boost::multiprecision::cpp_int(tx["value"].get<std::string>());
    std::cout << "[signTransaction] value = " << value << "\n";

    std::cout << "[signTransaction] Accessing data...\n";
    if (!tx.contains("data"))
        throw std::runtime_error("Missing field: data");
    auto dataBytes = hexToBytes(tx["data"].get<std::string>());
    std::cout << "[signTransaction] data bytes size = " << dataBytes.size() << "\n";

    std::cout << "[signTransaction] Accessing chainId...\n";
    if (!tx.contains("chainId"))
        throw std::runtime_error("Missing field: chainId");
    auto chainId = boost::multiprecision::cpp_int(tx["chainId"].get<std::string>());
    std::cout << "[signTransaction] chainId = " << chainId << "\n";

    std::cout << "[signTransaction] Converting private key hex to bytes...\n";
    std::string cleanPrivKey = privKeyHex;
    if (cleanPrivKey.rfind("0x", 0) == 0)
        cleanPrivKey = cleanPrivKey.substr(2);
    if (cleanPrivKey.length() != 64)
    {
        throw std::runtime_error("Private key must be 64 hex characters after 0x prefix removal, got length: " + std::to_string(cleanPrivKey.length()));
    }
    auto privKeyBytes = hexToBytes(cleanPrivKey);
    // auto privKeyBytes = hexToBytes(privKeyHex);
    std::cout << "[signTransaction] Private key bytes size = " << privKeyBytes.size() << "\n";

    if (privKeyBytes.size() != 32)
        throw std::runtime_error("Private key must be 32 bytes");

    std::cout << "[signTransaction] RLP encoding unsigned transaction...\n";

    std::vector<std::vector<uint8_t>> fields = {
        rlp_encode_integer(nonce),
        rlp_encode_integer(gasPrice),
        rlp_encode_integer(gasLimit),
        rlp_encode_string(toBytes),
        rlp_encode_integer(value),
        rlp_encode_string(dataBytes),
        rlp_encode_integer(chainId),
        rlp_encode_integer(0),
        rlp_encode_integer(0)};

    auto unsignedTx = rlp_encode_list(fields);

    std::cout << "[signTransaction] RLP unsigned tx size = " << unsignedTx.size() << "\n";
    std::cout << "[signTransaction] RLP unsigned tx (hex) = " << bytesToHex(unsignedTx.data(), unsignedTx.size()) << "\n";

    std::cout << "[signTransaction] Hashing unsigned transaction with keccak256...\n";
    auto hash = keccak256(unsignedTx);
    std::cout << "[signTransaction] Hash size = " << hash.size() << "\n";

    std::cout << "[signTransaction] Initializing secp256k1 context and signing...\n";
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_ecdsa_recoverable_signature sig;

    if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, hash.data(), privKeyBytes.data(), nullptr, nullptr))
    {
        secp256k1_context_destroy(ctx);
        throw std::runtime_error("Signing failed");
    }
    std::cout << "[signTransaction] Signature created\n";

    unsigned char output64[64];
    int recid;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, output64, &recid, &sig);
    secp256k1_context_destroy(ctx);

    std::cout << "[signTransaction] Serialized signature, recid = " << recid << "\n";

    std::vector<uint8_t> r(output64, output64 + 32);
    std::vector<uint8_t> s(output64 + 32, output64 + 64);

    boost::multiprecision::cpp_int v = recid + 35 + (chainId * 2);
    std::cout << "[signTransaction] Calculated v = " << v << "\n";

    std::cout << "[signTransaction] RLP encoding signed transaction...\n";
    std::vector<std::vector<uint8_t>> signedFields = {
        rlp_encode_integer(nonce),
        rlp_encode_integer(gasPrice),
        rlp_encode_integer(gasLimit),
        rlp_encode_string(toBytes),
        rlp_encode_integer(value),
        rlp_encode_string(dataBytes),
        rlp_encode_integer(v),
        rlp_encode_string(r),
        rlp_encode_string(s)};
    auto signedTx = rlp_encode_list(signedFields);
    std::cout << "[signTransaction] Signed tx size = " << signedTx.size() << "\n";

    std::string result = "0x" + bytesToHex(signedTx.data(), signedTx.size());
    std::cout << "[signTransaction] Signing process completed successfully\n";
    return result;
}

std::string signTransaction(
    const nlohmann::json &tx,
    const std::string &privateKey)
{
    return signTransactionImpl(tx, privateKey);
}
