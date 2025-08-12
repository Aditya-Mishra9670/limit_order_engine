#include "../blockchain/rpc_client.h"
#include "curve_pool.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <iostream> // for logging
#include "../orders/tif_policies.h"
#include "eth_utils.h"
#include "sign_utils.h" // <-- new include

// function selector for get_dy(int128,int128,uint256)
static const std::string GET_DY_SELECTOR = "5e0d443f"; // no 0x here
// selector for exchange(int128,int128,uint256,uint256)
static const std::string EXCHANGE_SELECTOR = "3df02124";

CurvePool::CurvePool(RPCClient *rpcClient, const std::string &poolAddress,
                     int inputIndex, int outputIndex,
                     int inputDecimals, int outputDecimals)
    : rpc(rpcClient), pool_address(poolAddress), i_index(inputIndex),
      j_index(outputIndex), input_decimals(inputDecimals), output_decimals(outputDecimals) {}

std::string CurvePool::strip0x(const std::string &s)
{
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return s.substr(2);
    return s;
}

std::string CurvePool::encodeUint256(const cpp_int &v)
{
    std::string hex;
    cpp_int tmp = v;
    if (tmp == 0)
        hex = "0";
    else
    {
        while (tmp > 0)
        {
            int digit = static_cast<int>(tmp & 0xF);
            char c = digit < 10 ? '0' + digit : 'a' + (digit - 10);
            hex.push_back(c);
            tmp >>= 4;
        }
        std::reverse(hex.begin(), hex.end());
    }
    if (hex.size() > 64)
        throw std::runtime_error("Value too large to encode as uint256");
    std::stringstream ss;
    ss << std::setw(64) << std::setfill('0') << hex;
    return ss.str();
}

cpp_int CurvePool::hexToCppInt(const std::string &hexIn)
{
    std::string s = strip0x(hexIn);
    cpp_int acc = 0;
    for (char ch : s)
    {
        acc *= 16;
        if (ch >= '0' && ch <= '9')
            acc += (ch - '0');
        else if (ch >= 'a' && ch <= 'f')
            acc += (10 + ch - 'a');
        else if (ch >= 'A' && ch <= 'F')
            acc += (10 + ch - 'A');
        else
            throw std::runtime_error("Invalid hex char in hexToCppInt");
    }
    return acc;
}

cpp_int CurvePool::getDyRaw(const cpp_int &dxSmall) const
{
    std::cout << "[getDyRaw] Preparing eth_call with dx = " << dxSmall.str() << std::endl;

    cpp_int i_val = i_index >= 0 ? cpp_int(i_index) : cpp_int(0);
    cpp_int j_val = j_index >= 0 ? cpp_int(j_index) : cpp_int(0);

    std::string data = "0x" + GET_DY_SELECTOR + encodeUint256(i_val) + encodeUint256(j_val) + encodeUint256(dxSmall);

    json params = json::array({json::object({{"to", pool_address}, {"data", data}}), "latest"});

    std::cout << "[getDyRaw] Sending eth_call with params: " << params.dump() << std::endl;

    json res = rpc->sendRequest("eth_call", params);

    std::cout << "[getDyRaw] eth_call response: " << res.dump(4) << std::endl;

    if (res.contains("error"))
    {
        std::string msg = "eth_call error: ";
        try
        {
            msg += res["error"]["message"].get<std::string>();
        }
        catch (...)
        {
            msg += "(unknown error message)";
        }
        throw std::runtime_error(msg);
    }

    if (!res.contains("result"))
    {
        std::stringstream ss;
        ss << "eth_call returned no 'result'. Full response: " << res.dump();
        throw std::runtime_error(ss.str());
    }

    std::string resultStr = res["result"].get<std::string>();
    std::cout << "[getDyRaw] Parsed result string: " << resultStr << std::endl;

    return hexToCppInt(resultStr);
}

long double CurvePool::getPriceForAmount(long double inputAmountHuman) const
{
    if (inputAmountHuman <= 0.0L)
        throw std::invalid_argument("inputAmountHuman must be > 0");

    long double scaleIn = std::pow(10.0L, input_decimals);
    cpp_int dxSmall = cpp_int(static_cast<unsigned long long>(inputAmountHuman * scaleIn));

    std::cout << "[getPriceForAmount] inputAmountHuman: " << inputAmountHuman << ", dxSmall: " << dxSmall.str() << std::endl;

    cpp_int dySmall = getDyRaw(dxSmall);

    long double dy_ld = dySmall.convert_to<long double>();
    long double dx_ld = dxSmall.convert_to<long double>();

    long double price = (dy_ld / dx_ld) * std::pow(10.0L, (long double)(input_decimals - output_decimals));
    std::cout << "[getPriceForAmount] price: " << price << std::endl;

    return price;
}

// ABI encode for exchange(int128,int128,uint256,uint256)
std::string CurvePool::encodeExchangeFunction(int128_t i, int128_t j,
                                              const cpp_int &dx,
                                              const cpp_int &min_dy)
{
    cpp_int i_val = cpp_int(i);
    cpp_int j_val = cpp_int(j);

    std::string encoded = "0x" + EXCHANGE_SELECTOR + encodeUint256(i_val) + encodeUint256(j_val) + encodeUint256(dx) + encodeUint256(min_dy);

    std::cout << "[encodeExchangeFunction] Encoded data: " << encoded << std::endl;

    return encoded;
}
std::string normalizeHexPrefix(const std::string& hexStr) {
    std::string s = hexStr;

    // Remove all "0x" or "0X" prefixes from the start repeatedly
    while (s.size() >= 2 && (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X")) {
        s = s.substr(2);
    }
    // Add single "0x" prefix
    return "0x" + s;
}

std::string CurvePool::exchange(int128_t i, int128_t j,
                                const cpp_int &dx,
                                const cpp_int &min_dy,
                                const std::string &fromAddress,
                                const std::string &privateKey)
{
    std::cout << "[exchange] Starting exchange function" << std::endl;

    std::string data = encodeExchangeFunction(i, j, dx, min_dy);

    std::cout << "[exchange] Fetching gas price..." << std::endl;
    json gasPriceResp = rpc->sendRequest("eth_gasPrice", json::array({}));
    std::cout << "[exchange] eth_gasPrice response: " << gasPriceResp.dump(4) << std::endl;

    if (!gasPriceResp.contains("result"))
    {
        throw std::runtime_error("eth_gasPrice response missing 'result'. Full response: " + gasPriceResp.dump());
    }
    cpp_int gasPrice = hexToCppInt(gasPriceResp["result"].get<std::string>());

    std::cout << "[exchange] Fetching nonce for address: " << fromAddress << std::endl;
    json nonceResp = rpc->sendRequest("eth_getTransactionCount", json::array({fromAddress, "pending"}));
    std::cout << "[exchange] eth_getTransactionCount response: " << nonceResp.dump(4) << std::endl;

    if (!nonceResp.contains("result"))
    {
        throw std::runtime_error("eth_getTransactionCount response missing 'result'. Full response: " + nonceResp.dump());
    }
    cpp_int nonce = hexToCppInt(nonceResp["result"].get<std::string>());

    json tx;
    tx["nonce"] = "0x" + cpp_int_to_hex(nonce);
    tx["gasPrice"] = "0x" + cpp_int_to_hex(gasPrice);
    tx["gas"] = "0x186A0"; // 100000 gas, adjust as needed
    tx["to"] = pool_address;
    tx["value"] = "0x0";
    tx["data"] = data;
    tx["chainId"] = "1"; // Sepolia chain ID

    std::cout << "[exchange] Signing transaction..." << std::endl;
    std::string signedTx = signTransaction(tx, privateKey);
    std::cout << "[exchange] Transaction signed." << std::endl;

    // json sendParams = {signedTx};
    // std::string signedTx = /* your raw tx hex string */;
    std::cout << "signedTx = '" << signedTx << "'" << std::endl;

    
    std::string normalizedTx = normalizeHexPrefix(signedTx);
    json sendParams = json::array({normalizedTx});

    std::cout << "[exchange] Sending raw transaction: " << signedTx << std::endl;

    json res = rpc->sendRequest("eth_sendRawTransaction", sendParams);

    std::cout << "[exchange] eth_sendRawTransaction response: " << res.dump(4) << std::endl;

    if (res.contains("error"))
    {
        std::string errMsg = "Transaction failed: ";
        try
        {
            errMsg += res["error"]["message"].get<std::string>();
        }
        catch (...)
        {
            errMsg += "(unknown error message)";
        }
        throw std::runtime_error(errMsg);
    }

    if (!res.is_string())
    {
        throw std::runtime_error("eth_sendRawTransaction response is not a string (tx hash). Full response: " + res.dump());
    }

    std::cout << "[exchange] Transaction hash: " << res.get<std::string>() << std::endl;

    return res.get<std::string>(); // tx hash
}
