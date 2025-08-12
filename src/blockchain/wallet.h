#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Wallet {
public:
    Wallet(const std::string& address, const std::string& privateKey, const std::string& rpcUrl);

    // Returns the ETH balance in wei as string
    std::string getBalance() const;

    // Returns the ETH balance in ether as double
    double getBalanceEther() const;

    const std::string& getAddress() const { return address; }

private:
    std::string address;
    std::string privateKey="e3915868765864c1e8dfef74c43313f36c778a86059161fb65344ef22332b656";
    std::string rpcUrl;

    nlohmann::json makeRpcRequest(const std::string& method, const nlohmann::json& params) const;
};
