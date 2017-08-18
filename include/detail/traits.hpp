#ifndef IMPL_PTR_DETAIL_TRAITS_HPP
#define IMPL_PTR_DETAIL_TRAITS_HPP

#include <boost/assert.hpp>

namespace detail
{
    // The incomplete-type management technique
    // is originally by Peter Dimov.

    template<typename, typename> struct traits;
    template<typename> struct      traits_base;
    template<typename T> using      traits_ptr = traits_base<T> const*;
}

template<typename impl_type>
struct detail::traits_base
{
    virtual ~traits_base() =default;

    virtual void   destroy (impl_type*) const =0;
    virtual void construct (void*,      impl_type const&) const { BOOST_ASSERT(0); }
    virtual void    assign (impl_type*, impl_type const&) const { BOOST_ASSERT(0); }

    virtual impl_type* copy_allocate (impl_type const*) const { BOOST_ASSERT(0); }
};

template<typename derived_type, typename impl_type>
struct detail::traits : detail::traits_base<impl_type>
{
    operator traits_base<impl_type> const*()
    {
        static derived_type trait; return &trait;
    }
};

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
