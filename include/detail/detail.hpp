// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_DETAIL_HPP
#define IMPL_PTR_DETAIL_DETAIL_HPP

#include <boost/type_traits.hpp>
#include <boost/assert.hpp>
#include <type_traits>
#include <memory>

#if 106500 <= BOOST_VERSION
#include <boost/core/pointer_traits.hpp>
#else

namespace boost
{
    template<typename T> T to_address(T p) { return p; }
}

#endif

namespace detail
{
    template<typename type1 =void,
             typename type2 =void,
             typename type3 =void,
             typename...>
    struct types
    {
        using  first_type = type1;
        using second_type = type2;
        using  third_type = type3;
    };

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
    using   alloc_type = typename allocator::template rebind<impl_type>::other;
    using alloc_traits = std::allocator_traits<alloc_type>;

    void destroy(impl_type* p) const override
    {
        alloc_type a;

        alloc_traits::destroy(a, p), a.deallocate(p, 1);
    }
};

template<typename impl_type, typename allocator>
struct detail::copyable_traits : detail::traits_base<copyable_traits<impl_type, allocator>, impl_type>
{
    using   alloc_type = typename allocator::template rebind<impl_type>::other;
    using alloc_traits = std::allocator_traits<alloc_type>;

    void
    destroy(impl_type* p) const override
    {
        alloc_type a;

        alloc_traits::destroy(a, p), a.deallocate(p, 1);
    }
    impl_type*
    construct(void* vp, impl_type const& from) const override
    {
        alloc_type  a;
        impl_type* ip = vp
                      ? static_cast<impl_type*>(vp)
                      : boost::to_address(a.allocate(1));

        alloc_traits::construct(a, ip, from);

        return ip;
    }
    void
    assign(impl_type* p, impl_type const& from) const override
    {
        *p = from;
    }
};

#endif // IMPL_PTR_DETAIL_DETAIL_HPP
