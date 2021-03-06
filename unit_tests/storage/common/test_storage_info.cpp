/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest/gtest.h"

#include <gridtools/common/gt_assert.hpp>
#include <gridtools/storage/common/storage_info.hpp>

using namespace gridtools;

TEST(StorageInfo, Strides) {
    {
        storage_info<0, layout_map<0, 1, 2>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 20);
        EXPECT_EQ((si.stride<1>()), 5);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.length()), 3 * 4 * 5);
    }

    {
        storage_info<0, layout_map<2, 0, 1>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 1);
        EXPECT_EQ((si.stride<1>()), 15);
        EXPECT_EQ((si.stride<2>()), 3);
        EXPECT_EQ((si.padded_total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.length()), 3 * 4 * 5);
    }
    {
        storage_info<0, layout_map<-1, 0, 1>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 0);
        EXPECT_EQ((si.stride<1>()), 5);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 4 * 5);
        EXPECT_EQ((si.total_length()), 4 * 5);
        EXPECT_EQ((si.length()), 4 * 5);
    }
}

TEST(StorageInfo, StridesAlignment) {
    {
        storage_info<0, layout_map<0, 1, 2>, halo<0, 0, 0>, alignment<32>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 128);
        EXPECT_EQ((si.stride<1>()), 32);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 3 * 4 * 32);
        EXPECT_EQ((si.total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.length()), 3 * 4 * 5);
    }

    {
        storage_info<0, layout_map<2, 0, 1>, halo<0, 0, 0>, alignment<32>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 1);
        EXPECT_EQ((si.stride<1>()), 32 * 5);
        EXPECT_EQ((si.stride<2>()), 32);
        EXPECT_EQ((si.padded_total_length()), 32 * 4 * 5);
        EXPECT_EQ((si.total_length()), 3 * 4 * 5);
        EXPECT_EQ((si.length()), 3 * 4 * 5);
    }
    {
        storage_info<0, layout_map<-1, 0, 1>, halo<0, 0, 0>, alignment<32>> si(3, 4, 5);

        EXPECT_EQ((si.stride<0>()), 0);
        EXPECT_EQ((si.stride<1>()), 32);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 4 * 32);
        EXPECT_EQ((si.total_length()), 4 * 5);
        EXPECT_EQ((si.length()), 4 * 5);
    }
}

TEST(StorageInfo, StridesAlignmentHalo) {
    {
        storage_info<0, layout_map<0, 1, 2>, halo<1, 2, 3>, alignment<32>> si(3, 5, 7);

        EXPECT_EQ((si.stride<0>()), 32 * 5);
        EXPECT_EQ((si.stride<1>()), 32);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 3 * 5 * 32);
        EXPECT_EQ((si.total_length()), 3 * 5 * 7);
        EXPECT_EQ((si.length()), 1);
    }

    {
        storage_info<0, layout_map<2, 0, 1>, halo<1, 2, 3>, alignment<32>> si(3, 5, 7);

        EXPECT_EQ((si.stride<0>()), 1);
        EXPECT_EQ((si.stride<1>()), 32 * 7);
        EXPECT_EQ((si.stride<2>()), 32);
        EXPECT_EQ((si.padded_total_length()), 32 * 5 * 7);
        EXPECT_EQ((si.total_length()), 3 * 5 * 7);
        EXPECT_EQ((si.length()), 1);
    }
    {
        storage_info<0, layout_map<-1, 0, 1>, halo<1, 2, 3>, alignment<32>> si(3, 5, 7);

        EXPECT_EQ((si.stride<0>()), 0);
        EXPECT_EQ((si.stride<1>()), 32);
        EXPECT_EQ((si.stride<2>()), 1);
        EXPECT_EQ((si.padded_total_length()), 5 * 32);
        EXPECT_EQ((si.total_length()), 5 * 7);
        EXPECT_EQ((si.length()), 1);
    }
}

TEST(StorageInfo, IndexVariadic) {
    {
        storage_info<0, layout_map<0, 1, 2>> si(3, 4, 5);

        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 1);
        EXPECT_EQ((si.index(0, 1, 0)), 5);
        EXPECT_EQ((si.index(1, 0, 0)), 20);
    }

    {
        storage_info<0, layout_map<2, 0, 1>> si(3, 4, 5);

        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 3);
        EXPECT_EQ((si.index(0, 1, 0)), 15);
        EXPECT_EQ((si.index(1, 0, 0)), 1);
    }
    {
        storage_info<0, layout_map<-1, 0, 1>> si(3, 4, 5);

        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 1);
        EXPECT_EQ((si.index(0, 1, 0)), 5);
        EXPECT_EQ((si.index(1, 0, 0)), 0);
        EXPECT_EQ((si.index(1, 1, 1)), 6);
    }
}

TEST(StorageInfo, Simple) {
    {
        storage_info<0, layout_map<2, 1, 0>> si(3, 3, 3);
        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 9);
        EXPECT_EQ((si.index(0, 0, 2)), 18);

        EXPECT_EQ((si.index(0, 1, 0)), 3);
        EXPECT_EQ((si.index(0, 1, 1)), 12);
        EXPECT_EQ((si.index(0, 1, 2)), 21);

        EXPECT_EQ((si.index(0, 2, 0)), 6);
        EXPECT_EQ((si.index(0, 2, 1)), 15);
        EXPECT_EQ((si.index(0, 2, 2)), 24);

        EXPECT_EQ((si.index(1, 0, 0)), 1);
        EXPECT_EQ((si.index(1, 0, 1)), 10);
        EXPECT_EQ((si.index(1, 0, 2)), 19);
    }
    {
        storage_info<0, layout_map<0, 1, 2>> si(3, 3, 3);
        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 1);
        EXPECT_EQ((si.index(0, 0, 2)), 2);

        EXPECT_EQ((si.index(0, 1, 0)), 3);
        EXPECT_EQ((si.index(0, 1, 1)), 4);
        EXPECT_EQ((si.index(0, 1, 2)), 5);

        EXPECT_EQ((si.index(0, 2, 0)), 6);
        EXPECT_EQ((si.index(0, 2, 1)), 7);
        EXPECT_EQ((si.index(0, 2, 2)), 8);

        EXPECT_EQ((si.index(1, 0, 0)), 9);
        EXPECT_EQ((si.index(1, 0, 1)), 10);
        EXPECT_EQ((si.index(1, 0, 2)), 11);
    }
    {
        storage_info<0, layout_map<1, 0, 2>> si(3, 3, 3);
        EXPECT_EQ((si.index(0, 0, 0)), 0);
        EXPECT_EQ((si.index(0, 0, 1)), 1);
        EXPECT_EQ((si.index(0, 0, 2)), 2);

        EXPECT_EQ((si.index(0, 1, 0)), 9);
        EXPECT_EQ((si.index(0, 1, 1)), 10);
        EXPECT_EQ((si.index(0, 1, 2)), 11);

        EXPECT_EQ((si.index(0, 2, 0)), 18);
        EXPECT_EQ((si.index(0, 2, 1)), 19);
        EXPECT_EQ((si.index(0, 2, 2)), 20);

        EXPECT_EQ((si.index(1, 0, 0)), 3);
        EXPECT_EQ((si.index(1, 0, 1)), 4);
        EXPECT_EQ((si.index(1, 0, 2)), 5);
    }

    // test wiht different dims
    storage_info<0, layout_map<1, 2, 3, 0>> x(5, 7, 8, 2);
    EXPECT_EQ((x.total_length<0>()), 5);
    EXPECT_EQ((x.total_length<1>()), 7);
    EXPECT_EQ((x.total_length<2>()), 8);
    EXPECT_EQ((x.total_length<3>()), 2);

    EXPECT_EQ((x.stride<0>()), 56);
    EXPECT_EQ((x.stride<1>()), 8);
    EXPECT_EQ((x.stride<2>()), 1);
    EXPECT_EQ((x.stride<3>()), 280);
}

TEST(StorageInfo, ArrayAccess) {
    {
        storage_info<0, layout_map<2, 1, 0>> si(3, 3, 3);
        EXPECT_EQ((si.index({0, 0, 0})), 0);
        EXPECT_EQ((si.index({0, 0, 1})), 9);
        EXPECT_EQ((si.index({0, 0, 2})), 18);

        EXPECT_EQ((si.index({0, 1, 0})), 3);
        EXPECT_EQ((si.index({0, 1, 1})), 12);
        EXPECT_EQ((si.index({0, 1, 2})), 21);

        EXPECT_EQ((si.index({0, 2, 0})), 6);
        EXPECT_EQ((si.index({0, 2, 1})), 15);
        EXPECT_EQ((si.index({0, 2, 2})), 24);

        EXPECT_EQ((si.index({1, 0, 0})), 1);
        EXPECT_EQ((si.index({1, 0, 1})), 10);
        EXPECT_EQ((si.index({1, 0, 2})), 19);
    }
    {
        storage_info<0, layout_map<0, 1, 2>> si(3, 3, 3);
        EXPECT_EQ((si.index({0, 0, 0})), 0);
        EXPECT_EQ((si.index({0, 0, 1})), 1);
        EXPECT_EQ((si.index({0, 0, 2})), 2);

        EXPECT_EQ((si.index({0, 1, 0})), 3);
        EXPECT_EQ((si.index({0, 1, 1})), 4);
        EXPECT_EQ((si.index({0, 1, 2})), 5);

        EXPECT_EQ((si.index({0, 2, 0})), 6);
        EXPECT_EQ((si.index({0, 2, 1})), 7);
        EXPECT_EQ((si.index({0, 2, 2})), 8);

        EXPECT_EQ((si.index({1, 0, 0})), 9);
        EXPECT_EQ((si.index({1, 0, 1})), 10);
        EXPECT_EQ((si.index({1, 0, 2})), 11);
    }
    {
        storage_info<0, layout_map<1, 0, 2>> si(3, 3, 3);
        EXPECT_EQ((si.index({0, 0, 0})), 0);
        EXPECT_EQ((si.index({0, 0, 1})), 1);
        EXPECT_EQ((si.index({0, 0, 2})), 2);

        EXPECT_EQ((si.index({0, 1, 0})), 9);
        EXPECT_EQ((si.index({0, 1, 1})), 10);
        EXPECT_EQ((si.index({0, 1, 2})), 11);

        EXPECT_EQ((si.index({0, 2, 0})), 18);
        EXPECT_EQ((si.index({0, 2, 1})), 19);
        EXPECT_EQ((si.index({0, 2, 2})), 20);

        EXPECT_EQ((si.index({1, 0, 0})), 3);
        EXPECT_EQ((si.index({1, 0, 1})), 4);
        EXPECT_EQ((si.index({1, 0, 2})), 5);
    }
}

TEST(StorageInfo, Halo) {
    // test with simple halo, dims and strides are extended
    storage_info<0, layout_map<2, 1, 0>, halo<2, 2, 2>> x(7, 7, 7);
    EXPECT_EQ((x.total_length<0>()), 7);
    EXPECT_EQ((x.total_length<1>()), 7);
    EXPECT_EQ((x.total_length<2>()), 7);

    EXPECT_EQ((x.stride<0>()), 1);
    EXPECT_EQ((x.stride<1>()), 7);
    EXPECT_EQ((x.stride<2>()), 49);

    // test with simple halo, dims and strides are extended
    storage_info<0, layout_map<0, 1, 2>, halo<2, 2, 2>> y(7, 7, 7);
    EXPECT_EQ((y.total_length<0>()), 7);
    EXPECT_EQ((y.total_length<1>()), 7);
    EXPECT_EQ((y.total_length<2>()), 7);

    EXPECT_EQ((y.stride<0>()), 49);
    EXPECT_EQ((y.stride<1>()), 7);
    EXPECT_EQ((y.stride<2>()), 1);

    // test with heterogeneous halo, dims and strides are extended
    storage_info<0, layout_map<2, 1, 0>, halo<2, 4, 0>> z(7, 11, 3);
    EXPECT_EQ((z.total_length<0>()), 7);
    EXPECT_EQ((z.total_length<1>()), 11);
    EXPECT_EQ((z.total_length<2>()), 3);

    EXPECT_EQ((z.stride<0>()), 1);
    EXPECT_EQ((z.stride<1>()), 7);
    EXPECT_EQ((z.stride<2>()), 77);
}

TEST(StorageInfo, Alignment) {
    {
        // test with different dims and alignment
        storage_info<0, layout_map<1, 2, 3, 0>, halo<0, 0, 0, 0>, alignment<32>> x(5, 7, 32, 2);
        EXPECT_EQ((x.total_length<0>()), 5);
        EXPECT_EQ((x.total_length<1>()), 7);
        EXPECT_EQ((x.total_length<2>()), 32);
        EXPECT_EQ((x.total_length<3>()), 2);

        EXPECT_EQ((x.stride<0>()), 32 * 7);
        EXPECT_EQ((x.stride<1>()), 32);
        EXPECT_EQ((x.stride<2>()), 1);
        EXPECT_EQ((x.stride<3>()), 5 * 32 * 7);
    }
    {
        // test with different dims, halo and alignment
        storage_info<0, layout_map<1, 2, 3, 0>, halo<1, 2, 3, 4>, alignment<32>> x(7, 11, 3, 10);
        EXPECT_EQ((x.total_length<0>()), 7);
        EXPECT_EQ((x.total_length<1>()), 11);
        EXPECT_EQ((x.total_length<2>()), 3);
        EXPECT_EQ((x.total_length<3>()), 10);

        EXPECT_EQ((x.stride<0>()), 32 * 11);
        EXPECT_EQ((x.stride<1>()), 32);
        EXPECT_EQ((x.stride<2>()), 1);
        EXPECT_EQ((x.stride<3>()), 32 * 11 * 7);

        EXPECT_EQ(x.index(0, 0, 0, 0), 0); // halo point
        EXPECT_EQ(x.index(0, 0, 1, 0), 1); // halo point
        EXPECT_EQ(x.index(0, 0, 2, 0), 2); // halo point
#ifndef NDEBUG
        EXPECT_THROW(x.index(0, 0, 3, 0), std::runtime_error); // first data point, aligned
#endif
    }
    {
        // test with different dims, halo and alignment
        storage_info<0, layout_map<3, 2, 1, 0>, halo<1, 2, 3, 4>, alignment<32>> x(3, 11, 14, 10);
        EXPECT_EQ((x.total_length<0>()), 3);
        EXPECT_EQ((x.total_length<1>()), 11);
        EXPECT_EQ((x.total_length<2>()), 14);
        EXPECT_EQ((x.total_length<3>()), 10);

        EXPECT_EQ((x.stride<0>()), 1);
        EXPECT_EQ((x.stride<1>()), 32);
        EXPECT_EQ((x.stride<2>()), 32 * 11);
        EXPECT_EQ((x.stride<3>()), 32 * 11 * 14);

        EXPECT_EQ(x.index(0, 0, 0, 0), 0); // halo point
        EXPECT_EQ(x.index(0, 1, 0, 0), 32);
        EXPECT_EQ(x.index(0, 0, 1, 0), 32 * 11);
        EXPECT_EQ(x.index(0, 0, 0, 1), 32 * 11 * 14);
    }
    {
        // test with masked dimensions
        storage_info<0, layout_map<1, -1, -1, 0>, halo<1, 2, 3, 4>, alignment<32>> x(7, 7, 8, 10);
        EXPECT_EQ((x.total_length<0>()), 7);
        EXPECT_EQ((x.total_length<1>()), 7);
        EXPECT_EQ((x.total_length<2>()), 8);
        EXPECT_EQ((x.total_length<3>()), 10);

        EXPECT_EQ((x.stride<0>()), 1);
        EXPECT_EQ((x.stride<1>()), 0);
        EXPECT_EQ((x.stride<2>()), 0);
        EXPECT_EQ((x.stride<3>()), 32);

        EXPECT_EQ(x.index(0, 0, 0, 0), 0); // halo point
        EXPECT_EQ(x.index(0, 1, 0, 0), 0);
        EXPECT_EQ(x.index(0, 0, 1, 0), 0);
        EXPECT_EQ(x.index(0, 0, 0, 1), 32);

        EXPECT_EQ(x.padded_total_length(), 32 * 10);
        EXPECT_EQ(x.total_length(), 7 * 10);
        EXPECT_EQ(x.length(), 5 * 2);
    }
}

TEST(StorageInfo, BeginEnd) {
    // no halo, no alignment
    storage_info<0, layout_map<1, 2, 0>> x(7, 8, 9);
    EXPECT_EQ(x.length(), 7 * 8 * 9);
    EXPECT_EQ(x.total_length(), 7 * 8 * 9);
    EXPECT_EQ(x.padded_total_length(), 7 * 8 * 9);
    EXPECT_EQ((x.begin<0>()), 0);
    EXPECT_EQ((x.end<0>()), 6);
    EXPECT_EQ((x.total_begin<0>()), 0);
    EXPECT_EQ((x.total_end<0>()), 6);
    EXPECT_EQ((x.begin<1>()), 0);
    EXPECT_EQ((x.end<1>()), 7);
    EXPECT_EQ((x.total_begin<1>()), 0);
    EXPECT_EQ((x.total_end<1>()), 7);
    EXPECT_EQ((x.begin<2>()), 0);
    EXPECT_EQ((x.end<2>()), 8);
    EXPECT_EQ((x.total_begin<2>()), 0);
    EXPECT_EQ((x.total_end<2>()), 8);

    EXPECT_EQ((x.length<0>()), 7);
    EXPECT_EQ((x.total_length<0>()), 7);
    EXPECT_EQ((x.length<1>()), 8);
    EXPECT_EQ((x.total_length<1>()), 8);
    EXPECT_EQ((x.length<2>()), 9);
    EXPECT_EQ((x.total_length<2>()), 9);

    // halo, no alignment
    storage_info<0, layout_map<1, 2, 0>, halo<1, 2, 3>> y(9, 11, 13);
    EXPECT_EQ(y.length(), 7 * 7 * 7);
    EXPECT_EQ(y.total_length(), 9 * 11 * 13);
    EXPECT_EQ(y.padded_total_length(), 9 * 11 * 13);

    EXPECT_EQ((y.begin<0>()), 1);
    EXPECT_EQ((y.end<0>()), 7);
    EXPECT_EQ((y.total_begin<0>()), 0);
    EXPECT_EQ((y.total_end<0>()), 8);

    EXPECT_EQ((y.begin<1>()), 2);
    EXPECT_EQ((y.end<1>()), 8);
    EXPECT_EQ((y.total_begin<1>()), 0);
    EXPECT_EQ((y.total_end<1>()), 10);

    EXPECT_EQ((y.begin<2>()), 3);
    EXPECT_EQ((y.end<2>()), 9);
    EXPECT_EQ((y.total_begin<2>()), 0);
    EXPECT_EQ((y.total_end<2>()), 12);

    EXPECT_EQ((y.length<0>()), 7);
    EXPECT_EQ((y.total_length<0>()), 9);
    EXPECT_EQ((y.length<1>()), 7);
    EXPECT_EQ((y.total_length<1>()), 11);
    EXPECT_EQ((y.length<2>()), 7);
    EXPECT_EQ((y.total_length<2>()), 13);

    // halo, alignment
    storage_info<0, layout_map<1, 2, 0>, halo<1, 2, 3>, alignment<16>> z(9, 11, 13);
    EXPECT_EQ(z.length(), 7 * 7 * 7);
    EXPECT_EQ(z.total_length(), 9 * 11 * 13);
    EXPECT_EQ(z.padded_total_length(), 9 * 16 * 13);
    EXPECT_EQ((z.begin<0>()), 1);
    EXPECT_EQ((z.end<0>()), 7);
    EXPECT_EQ((z.total_begin<0>()), 0);
    EXPECT_EQ((z.total_end<0>()), 8);

    EXPECT_EQ((y.begin<1>()), 2);
    EXPECT_EQ((y.end<1>()), 8);
    EXPECT_EQ((y.total_begin<1>()), 0);
    EXPECT_EQ((y.total_end<1>()), 10);

    EXPECT_EQ((y.begin<2>()), 3);
    EXPECT_EQ((y.end<2>()), 9);
    EXPECT_EQ((y.total_begin<2>()), 0);
    EXPECT_EQ((y.total_end<2>()), 12);

    EXPECT_EQ((y.length<0>()), 7);
    EXPECT_EQ((y.total_length<0>()), 9);
    EXPECT_EQ((y.length<1>()), 7);
    EXPECT_EQ((y.total_length<1>()), 11);
    EXPECT_EQ((y.length<2>()), 7);
    EXPECT_EQ((y.total_length<2>()), 13);
}

TEST(StorageInfo, Equal) {
    storage_info<0, layout_map<0, 1, 2>, halo<1, 2, 3>, alignment<16>> si1(9, 11, 13);
    storage_info<0, layout_map<0, 1, 2>, halo<1, 2, 3>, alignment<16>> si2(9, 11, 13);
    ASSERT_EQ(si1, si2);
}

TEST(StorageInfo, SizesNotEqual) {
    storage_info<0, layout_map<0, 1, 2>, halo<1, 2, 3>, alignment<16>> si1(9, 11, 13);
    storage_info<0, layout_map<0, 1, 2>, halo<1, 2, 3>, alignment<16>> si2(9, 11, 15);
    ASSERT_NE(si1, si2);
}
