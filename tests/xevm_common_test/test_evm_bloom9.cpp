// Copyright (c) 2017-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xevm_common/xbloom9.h"

#include <gtest/gtest.h>

NS_BEG3(top, evm_common, tests)

using top::xbytes_t;
using top::evm_common::xbloom9_t;

#define CON(x) (std::isdigit((x)) ? ((x) - '0') : (std::tolower((x)) - 'W'))
extern xbytes_t hex_string_to_bytes(std::string const & input) {
    xbytes_t result;
    assert(input.size() % 2 == 0);
    std::string input_data;
    if (input.size() > 2 && input.substr(0, 2) == "0x") {
        input_data = input.substr(2);
    } else {
        input_data = input;
    }
    result.resize(input_data.size() / 2);
    for (std::size_t i = 0; i < input_data.size() / 2; ++i) {
        result[i] = (CON(input_data[2 * i]) << 4) + CON(input_data[2 * i + 1]);
    }
    return result;
}

TEST(test_bloom, _0) {
    xbloom9_t bin;

    auto address = hex_string_to_bytes("e71d898e741c743326bf045959221cc39e0718d2");
    bin.add(address);

    auto data1 = hex_string_to_bytes("342827c97908e5e2f71151c08502a66d44b6f758e3ac2f1de95f02eb95f0a735");
    auto data2 = hex_string_to_bytes("0000000000000000000000000000000000000000000000000000000000000000");
    auto data3 = hex_string_to_bytes("000000000000000000000000047b2c1e1e9258ca2a7549d5b7987096f55109d1");

    auto data_false = hex_string_to_bytes("12345678");

    bin.add(data1);
    bin.add(data2);
    bin.add(data3);

    ASSERT_EQ(bin.get_hex_string_data(),
              "00000000000000000000000000000000000000000400000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000000800000000000"
              "00000000000000000000000000400000000002000000000000000000080000000000000000000000000000000000000001000020000000040000000000000000000000000000000000000000000000000001"
              "00000000000000000400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000"
              "00000000000000000000");

    ASSERT_TRUE(bin.contain(address));
    ASSERT_TRUE(bin.contain(data1));
    ASSERT_TRUE(bin.contain(data2));
    ASSERT_TRUE(bin.contain(data3));
    ASSERT_FALSE(bin.contain(data_false));
}

TEST(test_bloom, _1) {
    xbloom9_t bin;

    auto address = hex_string_to_bytes("0x0000000000000000000000000000000000000100");
    bin.add(address);

    auto data1 = hex_string_to_bytes("0x591bae32fbc801fec820cde2a1ac597eb40c47c67d087a10cc51417dad79e1a8");
    auto data2 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000001");
    auto data3 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000001");

    auto data_false = hex_string_to_bytes("12345678");

    bin.add(data1);
    bin.add(data2);
    bin.add(data3);

    ASSERT_EQ(bin.get_hex_string_data(),
              "00000000000000000000000000000000040000000000000000000002000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000"
              "00000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000010000010000000000000000000000000000000000000"
              "00000000000000000000000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000"
              "00000000000000000000");

    ASSERT_TRUE(bin.contain(address));
    ASSERT_TRUE(bin.contain(data1));
    ASSERT_TRUE(bin.contain(data2));
    ASSERT_TRUE(bin.contain(data3));
    ASSERT_FALSE(bin.contain(data_false));
}

TEST(test_bloom, _2) {
    xbloom9_t bin;

    auto address = hex_string_to_bytes("0x0000000000000000000000000000000000000100");
    bin.add(address);

    auto data1 = hex_string_to_bytes("0x591bae32fbc801fec820cde2a1ac597eb40c47c67d087a10cc51417dad79e1a8");
    auto data2 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000002");
    auto data3 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000001");

    auto data_false = hex_string_to_bytes("12345678");

    bin.add(data1);
    bin.add(data2);
    bin.add(data3);

    ASSERT_EQ(bin.get_hex_string_data(),
              "04000000000000000000000000000000040000000000000000000002000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000"
              "00000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000010000010000000000000000000000000000000000000"
              "00000000000001000000000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000080"
              "00000000000000000000");

    ASSERT_TRUE(bin.contain(address));
    ASSERT_TRUE(bin.contain(data1));
    ASSERT_TRUE(bin.contain(data2));
    ASSERT_TRUE(bin.contain(data3));
    ASSERT_FALSE(bin.contain(data_false));
}

TEST(test_bloom, _3) {
    xbloom9_t bin;

    auto address = hex_string_to_bytes("0x0000000000000000000000000000000000000100");
    bin.add(address);

    auto data1 = hex_string_to_bytes("0x591bae32fbc801fec820cde2a1ac597eb40c47c67d087a10cc51417dad79e1a8");
    auto data2 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000063");
    auto data3 = hex_string_to_bytes("0x0000000000000000000000000000000000000000000000000000000000000001");

    auto data_false = hex_string_to_bytes("12345678");

    bin.add(data1);
    bin.add(data2);
    bin.add(data3);

    ASSERT_EQ(bin.get_hex_string_data(),
              "00000000000000000000000000000000040000000000000000000002000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000"
              "00000004000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000080000000000000000000010000010000000000000000000000000000000000000"
              "00000000000000000000000004000000000000008000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000020000000"
              "00000000000000000000");

    ASSERT_TRUE(bin.contain(address));
    ASSERT_TRUE(bin.contain(data1));
    ASSERT_TRUE(bin.contain(data2));
    ASSERT_TRUE(bin.contain(data3));
    ASSERT_FALSE(bin.contain(data_false));
}

NS_END3