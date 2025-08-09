#pragma once
#include <string>

namespace Config {
    // RPC & Network
    extern std::string RPC_URL;
    extern std::string NETWORK_ID;

    // Wallet
    extern std::string PRIVATE_KEY;

    // Transaction Defaults
    extern int GAS_LIMIT;
    extern int GAS_PRICE_GWEI;

    // Curve Contract
    extern std::string CURVE_POOL_ADDRESS;

    // Load from .env file or defaults
    void loadFromEnv();
}


namespace SETTINGS {
    extern int RPC_MAX_RETRIES;
    extern int RPC_RETRY_DELAY_MS;
    extern int RPC_TIMEOUT_SEC;
    extern std::string RPC_URL;

    void loadFromEnv();
}


