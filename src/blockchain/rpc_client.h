#pragma once
#include <string>
#include <nlohmann/json.hpp>

class RPCClient {
private:
    std::string rpcUrl;

public:
    explicit RPCClient(const std::string& url);

    // Send a JSON-RPC request and get JSON response
    nlohmann::json sendRequest(const std::string& method, const nlohmann::json& params);

    // Convenience call to check connectivity
    uint64_t getLatestBlockNumber();
};
