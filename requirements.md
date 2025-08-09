[# Hackathon Challenge: Building a Curve DEX Limit Order Agent

**Objective:** Extend a basic C++ swap example to create a sophisticated off-chain agent that executes programmatic trades on the Curve DEX. The agent must manage orders with precise execution controls, including slippage protection and multiple Time-in-Force (TIF) policies (GTC, GTT, IOC, FOK).

**Background:**

While the Curve protocol executes swaps on-chain, sophisticated trading strategies require off-chain logic. A simple limit order is just the beginning. Professional trading agents must handle execution risk and time-sensitivity. This is achieved through:

*   **Slippage Protection:** Ensuring a trade executes within a tolerable price range, even in a volatile market. This is your safeguard against unfavorable price movement *during* transaction execution.
*   **Time-in-Force (TIF):** A set of rules that determine how long an order will remain active before it is executed or expires. Common policies include Good-Till-Canceled (GTC), Good-Till-Time (GTT), Immediate-Or-Cancel (IOC), and Fill-Or-Kill (FOK).

This challenge requires combining on-chain data queries with off-chain decision-making and robust transaction management.

**Resources:**

*   **Curve.fi Website:** [https://curve.fi/](https://curve.fi/)
*   **Technical Documentation:** [https://docs.curve.fi/](https://docs.curve.finance/)
*   **Curve GitHub (Smart Contracts):** [https://github.com/curvefi/curve-contract](https://github.com/curvefi/curve-contract)

**The Starter Code:**

You will begin with curve_swap_example.cpp. This file contains three basic examples of how to structure and execute different types of swaps against a Curve pool. Your first task is to understand this code and how it interacts with the blockchain.

**The Challenge is Divided into Three Parts:**

### Part 1: On-Chain Price Monitoring

Your first task is to build a reliable price feed by querying a Curve liquidity pool directly.

1.  **Choose an RPC Provider:** You must connect to the Sepolia testnet using a free and open JSON-RPC provider (e.g., Alchemy, Infura, or a public endpoint). You will need this to query balances and pool states.
2.  **Fetch Balances:** Demonstrate that you can use your RPC connection to fetch the balance of ETH and other ERC-20 tokens for a given wallet address.
3.  **Calculate the "Instant" Price:** Implement a function that repeatedly calls the get_dy or a similar "view" function on a Curve pool contract. This function will tell you, "If I put in X amount of token A, how much of token B will I get out *right now*?" This is your real-time price feed. You will need to query this price in a loop.

### Part 2: Implement the Limit Order Agent

This is the core of the challenge. You will build the logic for the limit order execution, including TIF policies.

1.  **Define the Limit Order:** Your agent must accept the following parameters for an order:
    *   Input Token & Amount (e.g., 100 USDC)
    *   Output Token (e.g., DAI)
    *   **Limit Price:** The target price for the swap (e.g., "I will only swap if the price is 1.001 DAI per USDC or better").
    *   **Slippage Tolerance:** A percentage (e.g., 0.5%) that defines the minimum acceptable amount of output tokens (min_dy) relative to the *current market price* at the moment of execution. This protects against on-chain price changes while your transaction is in flight.
    *   **Time-in-Force Policy:** The agent must support GTC, GTT, IOC, and FOK policies.

2.  **Create the Execution Logic:** Write a flexible loop or execution function that handles all TIF policies:
    *   **GTC (Good-Till-Canceled):** The agent monitors the price indefinitely and executes when the limit price is met.
    *   **GTT (Good-Till-Time):** The agent monitors until a specified timestamp. If the price is met, it executes. If the time expires, the order is canceled.
    *   **IOC (Immediate-Or-Cancel):** The agent checks the price *once*. If the order can be *partially or fully* filled at the limit price or better, it executes an immediate swap for the fillable amount. Any unfilled portion is canceled.
    *   **FOK (Fill-Or-Kill):** The agent checks the price and the pool's liquidity *once*. The *entire* order must be fillable at the limit price or better in a single transaction. If not, the order is canceled and no swap is attempted.

3.  **Execute the Swap:** When the conditions for the price and TIF policy are met, your agent must:
    *   Use the appropriate swap function from the curve_swap_example.cpp starter code.
    *   Construct and sign the transaction.
    *   Send the transaction to the blockchain via your RPC provider.

### Part 3: Testing Suite

You must prove that your agent works as expected.

1.  **Unit Tests:** Create a testing suite for your codebase. This should include tests for your price calculation logic and other critical functions.
2.  **End-to-End Test:** Write a test that runs the entire flow:
    *   Starts the agent with a limit order that is slightly *out* of the money (i.e., the target price is not yet met).
    *   (In a real scenario, you'd wait for the market to move. For a test, you might manually execute a separate swap that moves the price in your favor).
    *   The test should assert that once the price is favorable, your agent successfully submits the transaction.

---

### **Bonus Task: Wallet Management & Security**

For this hackathon, you must demonstrate best practices for wallet security.

1.  **Create a New Wallet:** Do not use a personal or production wallet. Create a brand new, empty wallet specifically for this hackathon using a tool like MetaMask.
2.  **Fund via a Testnet Faucet:**
    *   Once you have your new wallet's address, get free Sepolia ETH from a public "faucet." This will provide the necessary funds to pay for gas fees on the testnet.
    *   Good faucets include: sepoliafaucet.com (Alchemy) or infura.io/faucet/sepolia.
    *   Explain in your submission how and why this process is critical for security, contrasting it with the risks of using a mainnet seed phrase.

**Submission:**

Submit your work through this Google Form: https://docs.google.com/forms/d/1SqkTwnfb-07G3VI-TLDrxH8tfrrBW2AmVwbCRcPPyso/edit

Your submission must be a comprehensive package that includes:

1.  **Video Demonstration:** A short video recording (5-10 minutes) where you:
    *   Walk through your C++ source code, explaining the key components and logic for handling TIF policies and slippage.
    *   Run your program live to demonstrate that it is functional, executing one or two order types (e.g., a GTC order that fills and an IOC order that cancels) on the Sepolia testnet.
2.  **Complete Codebase:** Your full C++ source code, including the testing suite.
3.  **README.md File:** A detailed README.md explaining:
    *   Which JSON-RPC provider you used.
    *   Clear instructions on how to compile and run your code and tests.
    *   The steps you took for the bonus task on wallet security.
    *   The Sepolia address of the wallet you used for the hackathon.
]