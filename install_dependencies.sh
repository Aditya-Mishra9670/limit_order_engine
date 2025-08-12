#!/bin/bash
set -e

echo "Updating package list..."
sudo apt update

echo "Installing required C++ dependencies..."
sudo apt install -y \
    nlohmann-json3-dev \
    libboost-all-dev \
    libsecp256k1-dev \
    libcurl4-openssl-dev

echo "All dependencies installed successfully!"
