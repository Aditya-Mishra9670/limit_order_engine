#include "rpc_client.h"
#include "curve_pool.h"
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include "../config/settings.h"
#include "../utils/logger.h"

using json = nlohmann::json;


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

RPCClient::RPCClient(const std::string& url) : rpcUrl(url) {}

json RPCClient::sendRequest(const std::string& method, const json& params) {
    int maxRetries = SETTINGS::RPC_MAX_RETRIES;
    int baseDelayMs = SETTINGS::RPC_RETRY_DELAY_MS;

    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            Logger::error("Failed to initialize CURL");
            throw std::runtime_error("Failed to initialize CURL");
        }

        json payload = {
            {"jsonrpc", "2.0"},
            {"method", method},
            {"params", params},
            {"id", 1}
        };

        std::string payloadStr = payload.dump();
        std::string responseStr;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, rpcUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, SETTINGS::RPC_TIMEOUT_SEC);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            try {
                return json::parse(responseStr);
            } catch (const std::exception& e) {
                Logger::error("Failed to parse JSON response: " + std::string(e.what()));
                throw;
            }
        } else {
            Logger::warn("RPC request failed (Attempt " + std::to_string(attempt) +
                         "/" + std::to_string(maxRetries) +
                         "): " + std::string(curl_easy_strerror(res)));

            if (attempt < maxRetries) {
                int delay = baseDelayMs * (1 << (attempt - 1)); // exponential backoff
                Logger::info("Retrying in " + std::to_string(delay) + " ms...");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            } else {
                throw std::runtime_error("CURL request failed after " + std::to_string(maxRetries) + " attempts");
            }
        }
    }

    throw std::runtime_error("RPC request retry loop exited unexpectedly");
}

uint64_t RPCClient::getLatestBlockNumber() {
    json response = sendRequest("eth_blockNumber", json::array());
    if (response.contains("result")) {
        return std::stoull(response["result"].get<std::string>(), nullptr, 16);
    }
    throw std::runtime_error("Invalid RPC response for eth_blockNumber");
}
