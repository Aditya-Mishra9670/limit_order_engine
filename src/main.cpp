#include "blockchain/rpc_client.h"
#include "blockchain/curve_pool.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <cstdlib> // for getenv

using boost::multiprecision::cpp_int;

int main() {
    // --- Step 1: Read environment variables ---
    const char* rpcUrl = std::getenv("ETH_RPC_URL");
    const char* fromAddress = std::getenv("WALLET_ADDRESS");
    const char* privateKey = std::getenv("PRIVATE_KEY");

    if (!rpcUrl || !fromAddress || !privateKey) {
        std::cerr << "Please set ETH_RPC_URL, WALLET_ADDRESS, and PRIVATE_KEY environment variables.\n";
        return 1;
    }

    // --- Step 2: Create RPC client ---
    RPCClient rpc(rpcUrl);

    // --- Step 3: Configure Curve Pool on Sepolia ---
    // Replace with your actual Sepolia Curve Pool address and token details
    std::string poolAddress = "0x8bAB6d1b75f19e9eD9fCe8b9BD338844fF79aE27";
    int inputIndex = 0;         // token index in the pool (e.g., 0 for DAI)
    int outputIndex = 1;        // token index in the pool (e.g., 1 for USDC)
    int inputDecimals = 18;     // decimals of the input token
    int outputDecimals = 18;    // decimals of the output token

    CurvePool pool(&rpc, poolAddress, inputIndex, outputIndex, inputDecimals, outputDecimals);

    // --- Step 4: Prepare swap parameters ---
    cpp_int dx = cpp_int("1000000000000000000"); // 1 token (with 18 decimals)
    cpp_int minDy = cpp_int("0"); // Minimum amount out (set to 0 for testing)

    try {
        // --- Step 5: Execute swap ---
        std::string txHash = pool.exchange(
            inputIndex,
            outputIndex,
            dx,
            minDy,
            fromAddress,
            privateKey
        );

        // --- Step 6: Print transaction hash ---
        std::cout << "✅ Transaction sent!\n";
        std::cout << "   Hash: " << txHash << "\n";
        std::cout << "🔍 Track on Sepolia Etherscan: https://sepolia.etherscan.io/tx/" << txHash << "\n";

    } catch (const std::exception &e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
