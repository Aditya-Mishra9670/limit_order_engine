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

    // Debug prints: check values are loaded (avoid printing privateKey in prod)
    std::cout << "Using ETH_RPC_URL: " << rpcUrl << "\n";
    std::cout << "Using WALLET_ADDRESS: " << fromAddress << "\n";
    privateKey = "e3915868765864c1e8dfef74c43313f36c778a86059161fb65344ef22332b656";
    std::cout << "Using PRIVATE_KEY: " << privateKey << "\n"; // avoid printing secrets!
    try {
        // --- Step 2: Create RPC client ---
        RPCClient rpc(rpcUrl);
        std::cout<<"RPC client created"<<std::endl;
        // Optional: test a simple RPC call here to validate connection
        // e.g. get latest block number or chain ID (depends on your RPCClient interface)
        auto blockNumber = rpc.getLatestBlockNumber();
        std::cout << "Connected! Current block number: " << blockNumber << "\n";

        // // --- Step 3: Configure Curve Pool on Sepolia ---
        std::string poolAddress = "0x8bAB6d1b75f19e9eD9fCe8b9BD338844fF79aE27";
        int inputIndex = 0;         
        int outputIndex = 1;        
        int inputDecimals = 18;     
        int outputDecimals = 18;    

        CurvePool pool(&rpc, poolAddress, inputIndex, outputIndex, inputDecimals, outputDecimals);
        std::cout<<"pool creation success"<<std::endl;
        // --- Step 4: Prepare swap parameters ---
        cpp_int dx("100000000000000"); // 1 token (18 decimals)
        cpp_int minDy("0");                // Minimum amount out

        std::cout<<"starting pool.exchange to execute swap"<<std::endl;

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

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
