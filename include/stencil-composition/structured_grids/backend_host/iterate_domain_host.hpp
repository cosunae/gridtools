#pragma once

#include "stencil-composition/iterate_domain_fwd.hpp"
#include "stencil-composition/iterate_domain.hpp"
#include "stencil-composition/iterate_domain_metafunctions.hpp"
#include "stencil-composition/iterate_domain_impl_metafunctions.hpp"


namespace gridtools {

    /**
     * @brief iterate domain class for the Host backend
     */
    template<template<class> class IterateDomainBase, typename IterateDomainArguments>
    class iterate_domain_host : public IterateDomainBase<iterate_domain_host<IterateDomainBase, IterateDomainArguments> > //CRTP
    {
    DISALLOW_COPY_AND_ASSIGN(iterate_domain_host);
    GRIDTOOLS_STATIC_ASSERT((is_iterate_domain_arguments<IterateDomainArguments>::value), "Internal error: wrong type");

    typedef IterateDomainBase<iterate_domain_host<IterateDomainBase, IterateDomainArguments> > super;
    typedef typename IterateDomainArguments::local_domain_t local_domain_t;
public:
    typedef typename super::data_pointer_array_t data_pointer_array_t;
    typedef typename super::strides_cached_t strides_cached_t;
    typedef boost::mpl::map0<> ij_caches_map_t;

    GT_FUNCTION
    explicit iterate_domain_host(local_domain_t const& local_domain)
        : super(local_domain), m_data_pointer(0), m_strides(0) {}

    void set_data_pointer_impl(data_pointer_array_t* RESTRICT data_pointer)
    {
        assert(data_pointer);
        m_data_pointer = data_pointer;
    }

    data_pointer_array_t& RESTRICT data_pointer_impl()
    {
        assert(m_data_pointer);
        return *m_data_pointer;
    }
    data_pointer_array_t const & RESTRICT data_pointer_impl() const
    {
        assert(m_data_pointer);
        return *m_data_pointer;
    }

    strides_cached_t& RESTRICT strides_impl()
    {
        assert(m_strides);
        return *m_strides;
    }

    strides_cached_t const & RESTRICT strides_impl() const
    {
        assert(m_strides);
        return *m_strides;
    }

    void set_strides_pointer_impl(strides_cached_t* RESTRICT strides)
    {
        assert(strides);
        m_strides = strides;
    }

    template <ushort_t Coordinate, typename Execution>
    GT_FUNCTION
    void increment_impl() {}

    template <ushort_t Coordinate>
    GT_FUNCTION
    void increment_impl(int_t steps) {}

    template <ushort_t Coordinate>
    GT_FUNCTION
    void initialize_impl() {}

    template<typename ReturnType, typename Accessor, typename StoragePointer>
    GT_FUNCTION
    ReturnType get_value_impl(StoragePointer RESTRICT & storage_pointer, const uint_t pointer_offset) const
    {
        GRIDTOOLS_STATIC_ASSERT((is_accessor<Accessor>::value), "Wrong type");

        return super::template get_gmem_value<ReturnType>(storage_pointer,pointer_offset);
    }


private:
    data_pointer_array_t* RESTRICT m_data_pointer;
    strides_cached_t* RESTRICT m_strides;
};

template<
    template<class> class IterateDomainBase, typename IterateDomainArguments>
struct is_iterate_domain<
    iterate_domain_host<IterateDomainBase, IterateDomainArguments>
> : public boost::mpl::true_{};

template<
    template<class> class IterateDomainBase,
    typename IterateDomainArguments
>
struct is_positional_iterate_domain<iterate_domain_host<IterateDomainBase, IterateDomainArguments> > :
    is_positional_iterate_domain<IterateDomainBase<iterate_domain_host<IterateDomainBase, IterateDomainArguments> > > {};

template<template<class> class IterateDomainBase, typename IterateDomainArguments>
struct iterate_domain_backend_id<iterate_domain_host<IterateDomainBase, IterateDomainArguments> >
{
    typedef enumtype::enum_type< enumtype::platform, enumtype::Host > type;
};

}  //namespace gridtools
