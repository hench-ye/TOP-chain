// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xbasic/xasio_io_context_wrapper.h"
#include "xbasic/xthreading/xbackend_thread.hpp"
#include "xbasic/xtimer_driver.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <system_error>

TEST(xbasic, timer_driver) {
    auto io_context_wrapper = std::make_shared<top::xasio_io_context_wrapper_t>();
    io_context_wrapper->async_start();

    auto timer_driver = std::make_shared<top::xtimer_driver_t>(io_context_wrapper);
    timer_driver->start();

    constexpr std::size_t loop_count = 10;
    std::size_t counter{ 0 };
    for (auto i = 0u; i < loop_count; ++i) {
        timer_driver->schedule(std::chrono::milliseconds{ 1000 * (i + 1) }, [&counter, i](std::error_code const & ec)
        {
            if (ec) {
                EXPECT_EQ(asio::error::operation_aborted, ec.value());
            } else {
                EXPECT_EQ(i, counter);
                ++counter;
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds{ 30 });

    EXPECT_EQ(loop_count, counter);

    timer_driver->stop();
    timer_driver.reset();
    io_context_wrapper->stop();
    io_context_wrapper.reset();
}
