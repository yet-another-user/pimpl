#ifndef IMPL_PTR_DETAIL_TRAITS_HPP
#define IMPL_PTR_DETAIL_TRAITS_HPP

#include <boost/assert.hpp>
#include <boost/utility.hpp>
#include <type_traits>
#include <memory>

namespace detail
{
    // The incomplete-type management technique
    // is originally by Peter Dimov.

    struct traits
    {
        template<class> struct         base;
        template<class> struct managed_type;
        template<class> struct    copy_type;
        template<class> struct onstack_type;

        template<template<typename> class, class> struct ptr;
    };
}

template<typename impl_type>
struct detail::traits::base
{
    virtual ~base() =default;

    virtual void    destroy (impl_type*) const =0;
    virtual impl_type* copy (impl_type const*) const { BOOST_ASSERT(0); }
    virtual void     assign (impl_type*&, impl_type const*) const { BOOST_ASSERT(0); }
};

template<template<class> class derived_type, class impl_type>
struct detail::traits::ptr : detail::traits::base<impl_type>
{
    operator base<impl_type> const*()
    {
        static derived_type<impl_type> trait; return &trait;
    }
};

template<typename impl_type>
struct detail::traits::managed_type : detail::traits::ptr<managed_type, impl_type>
{
    void destroy (impl_type* p) const { boost::checked_delete(p); }
};

template<typename impl_type>
struct detail::traits::onstack_type : detail::traits::ptr<onstack_type, impl_type>
{
    void destroy (impl_type* p) const { p->~impl_type(); }
};

template<typename impl_type>
struct detail::traits::copy_type : detail::traits::ptr<copy_type, impl_type>
{
    void    destroy (impl_type* p) const { boost::checked_delete(p); }
    impl_type* copy (impl_type const* p) const { return p ? new impl_type(*p) : nullptr; }

    void assign (impl_type*& a, impl_type const* b) const
    {
        /**/ if (a == b);
        else if (a && b) *a = *b;
        else             destroy(a), a = copy(b);
    }
};

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
