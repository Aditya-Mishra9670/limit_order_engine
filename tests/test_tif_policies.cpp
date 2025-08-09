#define CATCH_CONFIG_MAIN
// #include "curve_dex_limit_order_agent/external/Catch2/catch.hpp"
#include "catch.hpp"
#include "eth_utils.h"
#include "sign_utils.h"
#include "../src/orders/tif_policies.h"
#include "../src/orders/order.h"

// Minimal MockPool implementing only getPriceForAmount
class MockPool : public CurvePool {
public:
    MockPool() : CurvePool(nullptr, std::string(), 0, 1, 6, 18) {}
    // override getPriceForAmount by hiding; not ideal but for tests we just add a function
    long double getPriceForAmount(long double amount) const {
        // simple mock: price = 1.0 for amount <= 100, price decreases for larger amounts
        if (amount <= 100.0L) return 1.0L;
        return 1.0L - (amount - 100.0L) * 0.001L; // quickly declines
    }
};

TEST_CASE("FOK returns true when full order is fillable at limit", "[tif]") {
    MockPool pool;
    LimitOrder o;
    o.inputAmount = 10.0L;
    o.limitPrice = 0.9L;
    o.slippagePct = 0.005L;
    REQUIRE(checkFOK(&pool, o) == true);
}

TEST_CASE("FOK returns false when not fillable", "[tif]") {
    MockPool pool;
    LimitOrder o;
    o.inputAmount = 10000.0L; // large -> price falls below limit
    o.limitPrice = 0.99L;
    REQUIRE(checkFOK(&pool, o) == false);
}

TEST_CASE("IOC computes partial fillable amount", "[tif]") {
    MockPool pool;
    LimitOrder o;
    o.inputAmount = 200.0L;
    o.limitPrice = 0.995L; // only small part fillable
    long double fillable = computeMaxFillableIOC(&pool, o);
    REQUIRE(fillable > 0.0L);
    REQUIRE(fillable <= o.inputAmount);
}
