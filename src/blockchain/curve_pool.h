#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using json = nlohmann::json;
using boost::multiprecision::cpp_int;
using boost::multiprecision::int128_t;

class RPCClient; // forward declaration

class CurvePool {
public:
    // Constructor
    CurvePool(RPCClient* rpc, const std::string &poolAddress,
              int inputIndex, int outputIndex,
              int inputDecimals, int outputDecimals);

    // Returns raw dy (uint256) as big integer
    cpp_int getDyRaw(const cpp_int &dxSmall) const;

    // Returns price (output tokens per 1 input token) e.g. DAI per USDC
    long double getPriceForAmount(long double inputAmountHuman) const;

    // Exchange function that sends a transaction and returns transaction hash
    std::string exchange(int128_t i, int128_t j,
                         const cpp_int& dx,
                         const cpp_int& min_dy,
                         const std::string& fromAddress,
                         const std::string& privateKey);

    // Getter for pool address
    const std::string& address() const { return pool_address; }

    // Static helper to encode exchange function call data (ABI encoding)
    static std::string encodeExchangeFunction(int128_t i, int128_t j,
                                              const cpp_int& dx,
                                              const cpp_int& min_dy);

private:
    RPCClient* rpc;
    std::string pool_address;
    int i_index;
    int j_index;
    int input_decimals;
    int output_decimals;

    // Static helper to strip 0x prefix from hex strings
    static std::string strip0x(const std::string &s);

    // Static helper to encode cpp_int as 64-char hex string (uint256)
    static std::string encodeUint256(const cpp_int &v);

    // Static helper to convert hex string to cpp_int
    static cpp_int hexToCppInt(const std::string &hex);
};
