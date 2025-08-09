#include "slippage.h"
#include <stdexcept>

long double applySlippage(long double expectedOutputHuman, long double slippagePct) {
    if (expectedOutputHuman < 0.0L) throw std::invalid_argument("expectedOutputHuman must be >= 0");
    if (slippagePct < 0.0L) throw std::invalid_argument("slippagePct must be >= 0");
    return expectedOutputHuman * (1.0L - slippagePct);
}
