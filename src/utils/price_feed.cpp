// price_feed.cpp
#include "price_feed.h"
#include "blockchain/curve_pool.h" // adjust path if needed
#include <chrono>
#include <iostream>

PriceFeed::PriceFeed(CurvePool* p, long double amountToCheck, long double pollIntervalSec)
: pool(p), amount(amountToCheck), intervalSec(pollIntervalSec), cb(nullptr),
  running(false), latestPrice(0.0L) {}

PriceFeed::~PriceFeed() {
    stop();
}

void PriceFeed::start() {
    if (running.load()) return;
    running.store(true);
    worker = std::thread([this]{ loop(); });
}

void PriceFeed::stop() {
    if (!running.load()) return;
    running.store(false);
    if (worker.joinable()) worker.join();
}

void PriceFeed::setCallback(Callback c) {
    std::lock_guard<std::mutex> lk(mtx);
    cb = std::move(c);
}

long double PriceFeed::getLatestPrice() {
    std::lock_guard<std::mutex> lk(mtx);
    return latestPrice;
}

void PriceFeed::loop() {
    while (running.load()) {
        try {
            long double p = pool->getPriceForAmount(amount);
            {
                std::lock_guard<std::mutex> lk(mtx);
                latestPrice = p;
            }
            if (cb) cb(p);
        } catch (const std::exception &e) {
            std::cerr << "PriceFeed error: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(intervalSec*1000)));
    }
}
