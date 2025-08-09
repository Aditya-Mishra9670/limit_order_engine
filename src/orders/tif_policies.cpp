#include "tif_policies.h"
#include "../utils/logger.h"
#include <cmath>
#include <stdexcept>

// Helper small epsilon for binary search
static const long double EPS = 1e-12L;

bool checkFOK(CurvePool* pool, const LimitOrder& order) {
    if (!pool) throw std::invalid_argument("pool is null");
    // Query the pool price for the entire input amount
    long double p = pool->getPriceForAmount(order.inputAmount); // output-per-input
    return priceMeetsOrExceeds(p, order.limitPrice);
}

// Binary-search style search for maximum fillable amount (human units).
// We assume getPriceForAmount(amount) is monotonic in amount (typical for AMMs: price may move with amount).
long double computeMaxFillableIOC(CurvePool* pool, const LimitOrder& order) {
    if (!pool) throw std::invalid_argument("pool is null");
    long double left = 0.0L;
    long double right = order.inputAmount;
    long double best = 0.0L;

    // Quick check: if full order is ok, return full
    long double fullPrice = pool->getPriceForAmount(order.inputAmount);
    if (priceMeetsOrExceeds(fullPrice, order.limitPrice)) {
        return order.inputAmount;
    }

    // If smallest increment fails, return 0
    long double tiny = std::max(order.inputAmount * 1e-6L, 1e-9L);
    long double tinyPrice = pool->getPriceForAmount(tiny);
    if (!priceMeetsOrExceeds(tinyPrice, order.limitPrice)) {
        return 0.0L;
    }

    // Binary search with fixed iterations
    for (int iter = 0; iter < 60; ++iter) {
        long double mid = (left + right) / 2.0L;
        if (mid <= 0.0L) {
            left = mid;
            continue;
        }
        long double midPrice = pool->getPriceForAmount(mid);
        if (priceMeetsOrExceeds(midPrice, order.limitPrice)) {
            best = mid;
            left = mid;
        } else {
            right = mid;
        }
        if (right - left < EPS) break;
    }

    return best;
}
