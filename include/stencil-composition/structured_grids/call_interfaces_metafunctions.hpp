#pragma once

namespace gridtools {
    namespace _impl {
        /** Metafunction to compute the index of the first accessor in the
            list of accessors to be written.
        */
        template <typename Functor>
        struct _get_index_of_first_non_const {

            template <int I, int L, typename List>
            struct scan_for_index {
                using type = typename boost::mpl::if_
                    <typename is_accessor_readonly<typename boost::mpl::at_c<List, I>::type >::type,
                     typename scan_for_index<I+1, L, List>::type,
                     static_int<I>
                     >::type;
            };

            template <int I, typename List>
            struct scan_for_index<I, I, List> {
                using type = static_int<-1>;
            };

            static const int value = scan_for_index
                <0,
                 boost::mpl::size<typename Functor::arg_list>::value,
                 typename Functor::arg_list>::type::value;
        };

        /** Metafunction to check that there is only one
            written argument in the argument list of a stencil
            operator, so that it is legal to call it as a
            function.
        */
        template <typename Functor>
        struct can_be_a_function {

            template <typename CurrentCount, typename CurrentArg>
            struct count_if_written {
                typedef typename boost::mpl::if_
                <typename is_accessor_written<CurrentArg>::type,
                 CurrentCount,
                 static_int<CurrentCount::value+1>
                 >::type type;
            };

            typedef typename boost::mpl::fold
            <typename Functor::arg_list,
             static_int<0>,
             count_if_written<boost::mpl::_1, boost::mpl::_2>
             >::type type;

            static const bool value = type::value==1;
        };


        template <typename Index, typename List>
        struct is_non_accessor {
            typedef typename boost::mpl::contains<List, Index>::type type;
            static const bool value = type::value;
        };


        template <typename PArguments>
        struct insert_index_if_not_accessor {
            template <typename Index, typename CurrentState>
            struct apply {
                typedef typename boost::mpl::at<PArguments, static_uint<Index::value>>::type to_check;
                typedef typename boost::mpl::if_<
                    is_accessor<to_check>,
                    CurrentState,
                    typename boost::mpl::push_back<CurrentState, Index>::type
                    >::type type;
            };
        };


        template <typename Type>
        struct wrap_reference {
            Type * value;

            wrap_reference(Type & v) : value(&v) {}
        };

        template <typename ...Args> struct package_args;

        template <class First, typename ...Args>
        struct package_args<First, Args...>
        {
            typedef typename boost::mpl::if_c<
                is_accessor<First>::value,
                First,
                wrap_reference<First>
                >::type to_pack;
            typedef typename boost::mpl::push_front<
                typename package_args<Args...>::type, to_pack>::type type;
        };

        template <class T>
        struct package_args<T>
        {
            typedef boost::mpl::vector<T> type;
        };

        template <>
        struct package_args<>
        {
            typedef boost::mpl::vector<> type;
        };


    } // namespace _impl
} // namespace gridtools
