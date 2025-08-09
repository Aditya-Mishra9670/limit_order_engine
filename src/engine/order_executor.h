#pragma once
#include <vector>
#include <mutex>
#include <functional>
#include <memory>
#include "../orders/order.h"

class CurvePool;
class PriceFeed;
class RPCClient;
class Wallet;

class OrderExecutor {
public:
    // wallet may be nullptr in simulation mode (no signing / no broadcasting)
    OrderExecutor(CurvePool* pool, PriceFeed* feed, Wallet* wallet = nullptr);

    ~OrderExecutor();

    // Submit a limit order (returns orderId)
    std::string submitOrder(const LimitOrder& order);

    // Stop the executor
    void stop();

    // Execution mode: simulation (no send) vs live (sign + send)
    void setSimulationMode(bool sim);

private:
    void priceCallback(long double price);
    void processOrder(LimitOrder& order, long double currentPrice);

    CurvePool* pool;
    PriceFeed* feed;
    Wallet* wallet;
    std::vector<LimitOrder> orders;
    std::mutex mtx;
    bool running;
    bool simulationMode;
};
