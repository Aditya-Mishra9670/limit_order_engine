cmake_minimum_required(VERSION 3.12)
project(curve_dex_limit_order_agent)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all warnings in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# --- Dependencies ---
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenSSL)

# Attempt to find libsecp256k1 via find_library
find_library(SECP256K1_LIB NAMES secp256k1)
if (NOT SECP256K1_LIB)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECP256K1_PC secp256k1)
    if (SECP256K1_PC_FOUND)
        message(STATUS "Found libsecp256k1 via pkg-config")
        set(SECP256K1_LIB ${SECP256K1_PC_LIBRARIES})
        include_directories(${SECP256K1_PC_INCLUDE_DIRS})
        link_directories(${SECP256K1_PC_LIBRARY_DIRS})
    else()
        message(WARNING "libsecp256k1 not found; live signing will fail.")
    endif()
else()
    message(STATUS "Found libsecp256k1: ${SECP256K1_LIB}")
endif()

# --- Include directories ---
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR}
)

# --- keccak-tiny Static Library ---
add_library(keccak_tiny STATIC
    src/utils/keccak-tiny/keccak-tiny.c
)
target_include_directories(keccak_tiny PUBLIC src/utils/keccak-tiny)
set_source_files_properties(src/utils/keccak-tiny/keccak-tiny.c PROPERTIES LANGUAGE C)

# --- Utility Library ---
add_library(utils STATIC
    src/utils/sign_utils.cpp
    src/utils/eth_utils.cpp
    src/utils/logger.cpp
    src/utils/keccak-tiny/keccak-tiny.c 
    src/utils/keccak_wrapper.cpp
)

target_link_libraries(utils PRIVATE
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    keccak_tiny
)

if (SECP256K1_LIB)
    target_link_libraries(utils PRIVATE ${SECP256K1_LIB})
endif()

if (OpenSSL_FOUND)
    target_link_libraries(utils PRIVATE OpenSSL::Crypto)
endif()


# --- Main Executable ---
file(GLOB_RECURSE MAIN_SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE utils keccak_tiny)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# --- Catch2 (vendored header-only v2.13.10) ---
set(CATCH2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/Catch2)
if (NOT EXISTS "${CATCH2_INCLUDE_DIR}/catch.hpp")
    message(FATAL_ERROR "Catch2 v2 header not found. Please run:
        mkdir -p external/Catch2 &&
        wget https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp -O external/Catch2/catch.hpp")
endif()

# --- Test Executable ---
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_executable(tests_runner
    ${TEST_SOURCES}
    src/orders/tif_policies.cpp
    src/blockchain/curve_pool.cpp
    src/blockchain/rpc_client.cpp
    src/orders/order.cpp
    src/config/settings.cpp 
)

target_include_directories(tests_runner PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CATCH2_INCLUDE_DIR}
)

target_link_libraries(tests_runner PRIVATE
    utils
    keccak_tiny
    nlohmann_json::nlohmann_json
    ${CURL_LIBRARIES}
)

if (SECP256K1_LIB)
    target_link_libraries(tests_runner PRIVATE ${SECP256K1_LIB})
endif()

if (OpenSSL_FOUND)
    target_link_libraries(tests_runner PRIVATE OpenSSL::Crypto)
endif()

# --- Enable tests ---
enable_testing()
add_test(NAME unit_tests COMMAND tests_runner)
