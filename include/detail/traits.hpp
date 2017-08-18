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

    namespace traits
    {
        template<class T> struct     base;
        template<class T> struct   unique;
        template<class T> struct copyable;
        template<class T> struct  onstack;
        template<class T> using   pointer = base<T> const*;

        template<template<typename> class, class> struct ptr;
    };
}

template<typename impl_type>
struct detail::traits::base
{
    virtual ~base() =default;

    virtual void   destroy (impl_type*) const =0;
    virtual void construct (void*,      impl_type const&) const { BOOST_ASSERT(0); }
    virtual void    assign (impl_type*, impl_type const&) const { BOOST_ASSERT(0); }

    virtual impl_type* copy_allocate (impl_type const*) const { BOOST_ASSERT(0); }
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
struct detail::traits::unique : detail::traits::ptr<unique, impl_type>
{
    void destroy (impl_type* p) const override { boost::checked_delete(p); }
};

template<typename impl_type>
struct detail::traits::onstack : detail::traits::ptr<onstack, impl_type>
{
    void   destroy (impl_type* p) const override { if (p) p->~impl_type(); }
    void construct (void*      to, impl_type const& from) const override { ::new(to) impl_type(from); }
    void    assign (impl_type* to, impl_type const& from) const override { *to = from; }
};

template<typename impl_type>
struct detail::traits::copyable : detail::traits::ptr<copyable, impl_type>
{
    void   destroy (impl_type* p) const override { boost::checked_delete(p); }
    void construct (void*      to, impl_type const& from) const override { ::new(to) impl_type(from); }
    void    assign (impl_type* to, impl_type const& from) const override { *to = from; }

    impl_type* copy_allocate (impl_type const* p) const override { return p ? new impl_type(*p) : nullptr; }
};

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
