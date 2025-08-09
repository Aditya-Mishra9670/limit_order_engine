#pragma once
#include "order.h"
#include "../blockchain/curve_pool.h"

// Utilities to evaluate TIF policies using the CurvePool price functions (human units).

// For FOK: return true if the full inputAmount can be filled at or above limitPrice
bool checkFOK(CurvePool* pool, const LimitOrder& order);

// For IOC: returns the max fillable input amount (human units) that satisfies the limit price.
// If zero, nothing is fillable.
long double computeMaxFillableIOC(CurvePool* pool, const LimitOrder& order);

// Helper to check price condition: price >= limitPrice
inline bool priceMeetsOrExceeds(long double price, long double limitPrice) {
    return price >= limitPrice;
}
