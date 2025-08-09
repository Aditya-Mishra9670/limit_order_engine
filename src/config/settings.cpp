#include "settings.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <map>
#include <algorithm>
#include "../utils/logger.h"

namespace SETTINGS {
    int RPC_MAX_RETRIES = 3;
    int RPC_RETRY_DELAY_MS = 500;
    int RPC_TIMEOUT_SEC = 10;
    std::string RPC_URL = "http://127.0.0.1:8545";

    static std::map<std::string, std::string> envMap;

    static void trim(std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\n\r"));
        s.erase(s.find_last_not_of(" \t\n\r") + 1);
    }

    static void loadEnvFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) {
            Logger::warn(".env file not found: " + path);
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            trim(key);
            trim(value);
            envMap[key] = value;
        }
    }

    static std::string getEnvValue(const std::string& key, const std::string& def) {
        auto it = envMap.find(key);
        return (it != envMap.end()) ? it->second : def;
    }

    static int getEnvValueInt(const std::string& key, int def) {
        try {
            return std::stoi(getEnvValue(key, std::to_string(def)));
        } catch (...) {
            return def;
        }
    }

    void loadFromEnv() {
        loadEnvFile(".env");

        RPC_MAX_RETRIES = getEnvValueInt("RPC_MAX_RETRIES", RPC_MAX_RETRIES);
        RPC_RETRY_DELAY_MS = getEnvValueInt("RPC_RETRY_DELAY_MS", RPC_RETRY_DELAY_MS);
        RPC_TIMEOUT_SEC = getEnvValueInt("RPC_TIMEOUT_SEC", RPC_TIMEOUT_SEC);
        RPC_URL = getEnvValue("RPC_URL", RPC_URL);

        Logger::info("Config loaded from .env:");
        Logger::info("  RPC_URL           = " + RPC_URL);
        Logger::info("  RPC_MAX_RETRIES   = " + std::to_string(RPC_MAX_RETRIES));
        Logger::info("  RPC_RETRY_DELAY   = " + std::to_string(RPC_RETRY_DELAY_MS) + " ms");
        Logger::info("  RPC_TIMEOUT       = " + std::to_string(RPC_TIMEOUT_SEC) + " sec");
    }
}
