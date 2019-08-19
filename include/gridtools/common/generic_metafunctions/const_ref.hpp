/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <type_traits>

#include "../../meta/macros.hpp"
#include "../../meta/type_traits.hpp"

namespace gridtools {
    GT_META_LAZY_NAMESPACE {
        template <class T, class = void>
        struct const_ref : std::add_lvalue_reference<add_const_t<T>> {};

        template <class T>
        struct const_ref<T,
            enable_if_t<!std::is_reference<T>::value && std::is_trivially_copy_constructible<T>::value &&
                        sizeof(T) <= sizeof(add_pointer_t<T>)>> : std::add_const<T> {};
    }

#if !GT_BROKEN_TEMPLATE_ALIASES
    template <class T>
    using const_ref = typename lazy::const_ref<T>::type;
#endif
} // namespace gridtools