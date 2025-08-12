#include "wallet.h"
#include "../utils/logger.h"
#include <curl/curl.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using json = nlohmann::json;

Wallet::Wallet(const std::string& addr, const std::string& pk, const std::string& url)
    : address(addr), privateKey(pk), rpcUrl(url) {}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

nlohmann::json Wallet::makeRpcRequest(const std::string& method, const nlohmann::json& params) const {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init CURL");

    // Full JSON-RPC 2.0 request
    nlohmann::json requestBody = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params},
        {"id", 1}
    };

    std::string requestStr = requestBody.dump();
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, rpcUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestStr.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    //additional curls for improve reliability
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);           // timeout 10 seconds
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);     // follow redirects if any
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);     // verify SSL peer
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);     // verify SSL host


    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("RPC request failed: " + std::string(curl_easy_strerror(res)));
    }
    //for debugging
    std::cout << "Raw RPC response: " << response << std::endl;  // <=== Debug here

    try {
        return nlohmann::json::parse(response);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON RPC response: " + std::string(e.what()) + "\nRaw: " + response);
    }
}

std::string Wallet::getBalance() const {
    // Ensure address is lowercase and 0x-prefixed
    std::string addr = address;
    std::transform(addr.begin(), addr.end(), addr.begin(), ::tolower);
    if (addr.find("0x") != 0) {
        addr = "0x" + addr;
    }

    nlohmann::json params = nlohmann::json::array({ addr, "latest" });
    nlohmann::json res;

    try {
        res = makeRpcRequest("eth_getBalance", params);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RPC request failed: ") + e.what());
    }

    // Debug: print full JSON response on error
    if (res.contains("error")) {
        std::string raw_response = res.dump();
        std::string errMsg = res["error"].value("message", "Unknown error");
        throw std::runtime_error("RPC Error: " + errMsg + "\nFull response: " + raw_response);
    }

    if (!res.contains("result") || res["result"].is_null()) {
        std::string raw_response = res.dump();
        throw std::runtime_error("RPC response missing 'result'\nFull response: " + raw_response);
    }

    return res["result"].get<std::string>();
}


double Wallet::getBalanceEther() const {
    std::string hexBalance;
    try {
        hexBalance = getBalance();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to get balance: ") + e.what());
    }

    if (hexBalance.size() < 3 || hexBalance.substr(0, 2) != "0x") {
        throw std::runtime_error("Invalid hex balance format: " + hexBalance);
    }

    unsigned long long wei = 0;
    try {
        wei = std::stoull(hexBalance.substr(2), nullptr, 16);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse balance from hex string '") + hexBalance + "': " + e.what());
    }

    return static_cast<double>(wei) / 1e18;
}
