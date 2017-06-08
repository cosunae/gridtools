/*
  GridTools Libraries

  Copyright (c) 2017, ETH Zurich and MeteoSwiss
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#pragma once

#include "../common/defs.hpp"
#include "./empty_extent.hpp"

namespace gridtools {

    /** @brief internal struct to simplify the API when we pass arguments to the global_accessor ```operator()```

        \tparam GlobalAccessor the associated global_accessor
        \tparam Args the type of the arguments passed to the ```operator()``` of the global_accessor

        The purpose of this struct is to add a "state" to the global accessor, storing the arguments
        passed to it inside a tuple. The global_accessor_with_arguments is not explicitly instantiated by the user, it
       gets generated
        when calling the ```operator()``` on a global_accessor. Afterwards it is treated as an expression by
        the iterate_domain which contains an overload of ```operator()``` specialised for
       global_accessor_with_arguments.
     */
    template < typename GlobalAccessor, typename... Args >
    struct global_accessor_with_arguments {
      private:
        boost::fusion::vector< Args... > m_arguments;

      public:
        typedef GlobalAccessor super;
        typedef typename super::index_t index_t;

        GT_FUNCTION
        global_accessor_with_arguments(Args &&... args_) : m_arguments(std::forward< Args >(args_)...) {}
        GT_FUNCTION
        boost::fusion::vector< Args... > const &get_arguments() const { return m_arguments; };
    };

    /**
       @brief object to be accessed regardless the current iteration point

       \tparam I unique accessor identifier
       \tparam Intend the global accessors must me read-only

       This accessor allows the user to call a user function contained in a user-defined object.
       Calling the parenthesis operator on the global_accessor generates an instance of
       ```global_accessor_with_arguments```.
     */
    template < uint_t I, enumtype::intend Intend = enumtype::in >
    struct global_accessor {

        static const constexpr enumtype::intend intent = Intend;

        typedef global_accessor< I, Intend > type;

        typedef static_uint< I > index_t;

        typedef empty_extent extent_t;

        /** @brief generates a global_accessor_with_arguments and returns it by value */
        template < typename... Args >
        GT_FUNCTION global_accessor_with_arguments< global_accessor, Args... > operator()(Args &&... args_) {
            return global_accessor_with_arguments< global_accessor, Args... >(std::forward< Args >(args_)...);
        }
    };

    template < typename Type >
    struct is_global_accessor : boost::false_type {};

    template < uint_t I, enumtype::intend Intend >
    struct is_global_accessor< global_accessor< I, Intend > > : boost::true_type {};

    template < typename Global, typename... Args >
    struct is_global_accessor< global_accessor_with_arguments< Global, Args... > > : boost::true_type {};

    template < typename T >
    struct is_global_accessor_with_arguments : boost::false_type {};

    template < typename Global, typename... Args >
    struct is_global_accessor_with_arguments< global_accessor_with_arguments< Global, Args... > > : boost::true_type {};

} // namespace gridtools