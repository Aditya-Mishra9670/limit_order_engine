// price_feed.h
#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

class CurvePool; // forward

class PriceFeed {
public:
    using Callback = std::function<void(long double)>;

    PriceFeed(CurvePool* pool, long double amountToCheck = 1.0L, long double pollIntervalSec = 5.0L);
    ~PriceFeed();

    void start();
    void stop();
    void setCallback(Callback cb);

    long double getLatestPrice();

private:
    void loop();

private:
    CurvePool* pool;
    long double amount;
    long double intervalSec;
    Callback cb;

    std::thread worker;
    std::atomic<bool> running;
    std::mutex mtx;
    long double latestPrice;
};
