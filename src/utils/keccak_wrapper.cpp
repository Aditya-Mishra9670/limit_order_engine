#include "keccak-tiny/keccak-tiny.h"
#include "keccak_wrapper.h"
#include <vector>
#include <cstdint>
#include <stdexcept>

std::vector<uint8_t> keccak256(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> hash(32);
    int res = sha3_256(hash.data(), hash.size(), input.data(), input.size());
    if (res != 0) {
        throw std::runtime_error("sha3_256 hashing failed");
    }
    return hash;
}
