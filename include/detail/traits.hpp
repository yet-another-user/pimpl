// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_TRAITS_HPP
#define IMPL_PTR_DETAIL_TRAITS_HPP

#include <boost/assert.hpp>

namespace detail
{
    // The incomplete-type management technique
    // is originally by Peter Dimov.

    template<typename, typename> struct     traits_base;
    template<typename, typename> struct   unique_traits;
    template<typename, typename> struct copyable_traits;
}

template<typename traits_type, typename impl_type>
struct detail::traits_base
{
    using this_type = traits_base<traits_type, impl_type>;
    using   pointer = this_type const*;

    virtual ~traits_base() =default;

    virtual void         destroy (impl_type*) const =0;
    virtual void          assign (impl_type*, impl_type const&) const { BOOST_ASSERT(0); }
    virtual impl_type* construct (void*, impl_type const&) const { BOOST_ASSERT(0); return nullptr; }

    operator pointer()
    {
        static_assert(std::is_base_of<this_type, traits_type>::value, "");

        static traits_type const traits; return &traits;
    }
};

template<typename impl_type, typename allocator>
struct detail::unique_traits : detail::traits_base<unique_traits<impl_type, allocator>, impl_type>
{
    using alloc_type = typename allocator::template rebind<impl_type>::other;

    void destroy(impl_type* p) const override
    {
        alloc_type a;

        a.destroy(p);
        a.deallocate(p, 1);
    }
};

template<typename impl_type, typename allocator>
struct detail::copyable_traits : detail::traits_base<copyable_traits<impl_type, allocator>, impl_type>
{
    using alloc_type = typename allocator::template rebind<impl_type>::other;

    void
    destroy(impl_type* p) const override
    {
        alloc_type a;

        a.destroy(p);
        a.deallocate(p, 1);
    }
    impl_type*
    construct(void* p, impl_type const& from) const override
    {
        if (!p) p = alloc_type().allocate(1);
        return ::new(p) impl_type(from);
    }
    void
    assign(impl_type* p, impl_type const& from) const override
    {
        *p = from;
    }
};

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
