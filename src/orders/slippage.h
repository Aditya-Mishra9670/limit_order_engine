#pragma once
// Simple human-unit slippage helper

// expectedOutputHuman: expected output token amount (human units)
// slippagePct: 0.005 -> 0.5%
// returns the minimum acceptable output (human units) after applying slippage tolerance
long double applySlippage(long double expectedOutputHuman, long double slippagePct);
