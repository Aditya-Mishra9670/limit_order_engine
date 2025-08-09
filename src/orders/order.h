#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

struct LimitOrder {
    enum class TIF { GTC, GTT, IOC, FOK };
    enum class Status { PENDING, PARTIAL_FILLED, FILLED, CANCELED, FAILED };

    std::string orderId;
    std::string inputToken;   // address
    std::string outputToken;  // address
    long double inputAmount;  // human units (e.g., 100.0 USDC)
    long double limitPrice;   // output-per-input (e.g., DAI per USDC)
    long double slippagePct;  // 0.005 = 0.5%
    TIF tif;
    // expiry only used if tif == GTT
    std::chrono::system_clock::time_point expiry;
    Status status = Status::PENDING;
    long double filledAmount = 0.0L; // human units filled so far

    nlohmann::json toJson() const;
};
