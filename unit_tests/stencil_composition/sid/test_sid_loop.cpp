/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gridtools/stencil_composition/sid/loop.hpp>

#include <gtest/gtest.h>

#include <gridtools/common/compose.hpp>
#include <gridtools/common/integral_constant.hpp>
#include <gridtools/common/tuple.hpp>
#include <gridtools/common/tuple_util.hpp>
#include <gridtools/stencil_composition/sid/synthetic.hpp>

namespace gridtools {
    namespace {
        using sid::property;
        using namespace literals;
        namespace tu = tuple_util::host_device;

        struct assignment_f {
            double m_val;
            template <class Strides>
            GT_FUNCTION void operator()(double *ptr, Strides const &) const {
                *ptr = m_val;
            }
        };

        using i_t = integral_constant<int, 0>;
        using j_t = integral_constant<int, 1>;

        TEST(make_loop, smoke) {
            double data[10][10] = {};
            auto strides = tu::make<tuple>(10_c, 1_c);
            using strides_t = decltype(strides);

            double *ptr = &data[0][0];
            sid::make_loop<i_t>(5, 1)(assignment_f{42})(ptr, strides);
            for (int i = 0; i < 5; ++i)
                EXPECT_EQ(42, data[i][0]) << " i:" << i;

            ptr = &data[2][3];
            sid::make_loop<j_t>(4_c, -1_c)(assignment_f{5})(ptr, strides);
            for (int i = 0; i < 4; ++i)
                EXPECT_EQ(5, data[2][i]) << " i:" << i;

            ptr = &data[0][0];
            sid::make_loop<i_t>(10_c)(sid::make_loop<j_t>(10_c)(assignment_f{88}))(ptr, strides);
            for (int i = 0; i < 10; ++i)
                for (int j = 0; j < 10; ++j)
                    EXPECT_EQ(88, data[i][j]) << " i:" << i << ", j:" << j;
        }

        TEST(nest_loops, smoke) {
            double data[10][10] = {};
            double *ptr = &data[0][0];
            auto strides = tu::make<tuple>(10_c, 1_c);
            using strides_t = decltype(strides);

            auto testee = host_device::compose(sid::make_loop<i_t>(10_c), sid::make_loop<j_t>(10_c));

            testee(assignment_f{42})(ptr, strides);

            for (int i = 0; i < 10; ++i)
                for (int j = 0; j < 10; ++j)
                    EXPECT_EQ(42, data[i][j]) << " i:" << i << ", j:" << j;
        }

        TEST(range, smoke) {
            double data[10][10] = {};
            double *ptr = &data[0][0];
            auto strides = tu::make<tuple>(10_c, 1_c);

            auto testee = sid::make_range(ptr, strides, sid::make_loop<i_t>(10_c), sid::make_loop<j_t>(10_c));

            for (auto &&val : testee)
                val = 42;

            for (int i = 0; i < 10; ++i)
                for (int j = 0; j < 10; ++j)
                    EXPECT_EQ(42, data[i][j]) << " i:" << i << ", j:" << j;
        }
    } // namespace
} // namespace gridtools