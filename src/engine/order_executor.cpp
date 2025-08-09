#include "order_executor.h"
#include "utils/price_feed.h"
#include "../orders/tif_policies.h"
#include "../orders/slippage.h"
#include "../utils/logger.h"
#include "../blockchain/curve_pool.h"
#include <chrono>
#include <sstream>
#include <uuid/uuid.h> // optional, we will generate simple id if uuid not available
#include <iostream>

// Helper to generate a quick unique id (fallback)
static std::string makeOrderId() {
    static uint64_t counter = 1;
    std::ostringstream ss;
    ss << "ord-" << std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) << "-" << counter++;
    return ss.str();
}

OrderExecutor::OrderExecutor(CurvePool* p, PriceFeed* f, Wallet* w)
: pool(p), feed(f), wallet(w), running(true), simulationMode(true) {
    if (feed) {
        // subscribe callback
        feed->setCallback([this](long double price){ this->priceCallback(price); });
    }
}

OrderExecutor::~OrderExecutor() {
    stop();
}

std::string OrderExecutor::submitOrder(const LimitOrder& orderIn) {
    std::lock_guard<std::mutex> lk(mtx);
    LimitOrder ord = orderIn;
    if (ord.orderId.empty()) ord.orderId = makeOrderId();
    orders.push_back(ord);
    Logger::info("Submitted order: " + ord.orderId);
    return ord.orderId;
}

void OrderExecutor::stop() {
    running = false;
    if (feed) feed->stop();
}

void OrderExecutor::setSimulationMode(bool sim) {
    simulationMode = sim;
}

void OrderExecutor::priceCallback(long double price) {
    // Called by PriceFeed on every update
    std::lock_guard<std::mutex> lk(mtx);
    for (auto &ord : orders) {
        if (ord.status == LimitOrder::Status::PENDING) {
            processOrder(ord, price);
        }
    }
}

void OrderExecutor::processOrder(LimitOrder& order, long double currentPrice) {
    try {
        // Check expiry for GTT
        if (order.tif == LimitOrder::TIF::GTT) {
            auto now = std::chrono::system_clock::now();
            if (now >= order.expiry) {
                order.status = LimitOrder::Status::CANCELED;
                Logger::info("Order expired (GTT): " + order.orderId);
                return;
            }
        }

        // Price must meet or exceed limit to trigger
        if (!priceMeetsOrExceeds(currentPrice, order.limitPrice)) {
            return;
        }

        // Handle by TIF
        if (order.tif == LimitOrder::TIF::FOK) {
            bool ok = checkFOK(pool, order);
            if (!ok) {
                order.status = LimitOrder::Status::CANCELED;
                Logger::info("FOK not satisfied, canceling: " + order.orderId);
                return;
            }

            // Prepare min_dy and execute full swap
            long double expectedOutput = pool->getPriceForAmount(order.inputAmount) * order.inputAmount;
            long double minOutput = applySlippage(expectedOutput, order.slippagePct);

            if (simulationMode) {
                Logger::success("SIM: FOK executing full swap for " + order.orderId);
                std::string tx = pool->exchange(0,1,(uint64_t)(order.inputAmount), (uint64_t)minOutput, /*receiver*/"", /*priv*/"");
                (void)tx;
                order.status = LimitOrder::Status::FILLED;
                order.filledAmount = order.inputAmount;
            } else {
                // TODO: build tx, sign and send
                Logger::info("LIVE: FOK execution - SIGN & SEND not yet implemented");
            }
            return;
        } else if (order.tif == LimitOrder::TIF::IOC) {
            long double fillable = computeMaxFillableIOC(pool, order);
            if (fillable <= 0.0L) {
                order.status = LimitOrder::Status::CANCELED;
                Logger::info("IOC not fillable, cancel: " + order.orderId);
                return;
            }
            long double expectedOutput = pool->getPriceForAmount(fillable) * fillable;
            long double minOutput = applySlippage(expectedOutput, order.slippagePct);
            if (simulationMode) {
                Logger::success("SIM: IOC executing partial swap for order " + order.orderId + " amount " + std::to_string(fillable));
                std::string tx = pool->exchange(0,1,(uint64_t)(fillable), (uint64_t)minOutput, /*receiver*/"", /*priv*/"");
                (void)tx;
                order.filledAmount += fillable;
                order.status = LimitOrder::Status::PARTIAL_FILLED;
            } else {
                Logger::info("LIVE: IOC execution - SIGN & SEND not yet implemented");
            }
            return;
        } else {
            // GTC / GTT - treat similarly: execute full or partial per price
            long double expectedOutput = pool->getPriceForAmount(order.inputAmount) * order.inputAmount;
            long double minOutput = applySlippage(expectedOutput, order.slippagePct);
            if (simulationMode) {
                Logger::success("SIM: GTC/GTT executing swap for " + order.orderId);
                std::string tx = pool->exchange(0,1,(uint64_t)(order.inputAmount), (uint64_t)minOutput, /*receiver*/"", /*priv*/"");
                (void)tx;
                order.filledAmount = order.inputAmount;
                order.status = LimitOrder::Status::FILLED;
            } else {
                Logger::info("LIVE: GTC/GTT execution - SIGN & SEND not yet implemented");
            }
            return;
        }
    } catch (const std::exception &e) {
        order.status = LimitOrder::Status::FAILED;
        Logger::error(std::string("Error processing order ") + order.orderId + ": " + e.what());
    }
}
