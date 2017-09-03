// Copyright (c) 2008 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_DETAIL_HPP
#define IMPL_PTR_DETAIL_DETAIL_HPP

#include <boost/type_traits.hpp>
#include <boost/assert.hpp>
#include <type_traits>
#include <memory>

#if 106500 <= BOOST_VERSION
#   include <boost/core/pointer_traits.hpp>
#else
    namespace boost
    {
        template<typename T> T to_address(T p) { return p; }
    }
#endif

namespace detail
{
    template<typename>
    struct     no_policy {};
    struct in_place_type {};

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
    struct traits
    {
        template<typename, typename> struct     base;
        template<typename, typename> struct   unique;
        template<typename, typename> struct copyable;
    };
}

template<typename traits_type, typename impl_type>
struct detail::traits::base
{
    using this_type = base<traits_type, impl_type>;
    using   pointer = this_type const*;

    virtual ~base() =default;

    virtual void         destroy (impl_type*) const =0;
    virtual void          assign (impl_type*, impl_type const&) const { BOOST_ASSERT(!"not implemented"); }
    virtual void          assign (impl_type* p, impl_type&& from) const { assign(p, from); }
    virtual void       construct (void*, impl_type const&) const { BOOST_ASSERT(!"not implemented"); }
    virtual void       construct (void* p, impl_type&& from) const { return construct(p, from); }
    virtual impl_type*      make (impl_type const&) const { BOOST_ASSERT(!"not implemented"); return nullptr; }
    virtual impl_type*      make (impl_type&& from) const { return make(from); }

    static pointer singleton()
    {
        static_assert(!std::is_same<this_type, traits_type>::value, "");
        static_assert(std::is_base_of<this_type, traits_type>::value, "");

        static traits_type const traits; return &traits;
    }
};

template<typename impl_type, typename allocator>
struct detail::traits::unique final : base<unique<impl_type, allocator>, impl_type>
{
    using   alloc_type = typename std::allocator_traits<allocator>::template rebind_alloc<impl_type>;
    using alloc_traits = std::allocator_traits<alloc_type>;

    void destroy(impl_type* p) const override
    {
        alloc_type a;

        alloc_traits::destroy(a, p);
        alloc_traits::deallocate(a, p, 1);
    }
};

template<typename impl_type, typename allocator>
struct detail::traits::copyable final : base<copyable<impl_type, allocator>, impl_type>
{
    using   alloc_type = typename std::allocator_traits<allocator>::template rebind_alloc<impl_type>;
    using alloc_traits = std::allocator_traits<alloc_type>;

    void destroy(impl_type* p) const override
    {
        alloc_type a;

        alloc_traits::destroy(a, p);
        alloc_traits::deallocate(a, p, 1);
    }
    void construct(void* vp, impl_type const& from) const override
    {
        alloc_type        a;
        impl_type* const ip = static_cast<impl_type*>(vp);
        alloc_traits::construct(a, ip, from);
    }
    void construct(void* vp, impl_type&& from) const override
    {
        alloc_type        a;
        impl_type* const ip = static_cast<impl_type*>(vp);
        alloc_traits::construct(a, ip, std::move(from));
    }
    impl_type* make(impl_type const& from) const override
    {
        alloc_type  a;
        const auto ip = alloc_traits::allocate(a, 1);

        try
        {
            alloc_traits::construct(a, boost::to_address(ip), from);
        }
        catch (...)
        {
            alloc_traits::deallocate(a, ip, 1);
            throw;
        }

        return boost::to_address(ip);
    }
    impl_type* make(impl_type&& from) const override
    {
        alloc_type  a;
        const auto ip = alloc_traits::allocate(a, 1);

        try
        {
            alloc_traits::construct(a, boost::to_address(ip), std::move(from));
        }
        catch (...)
        {
            alloc_traits::deallocate(a, ip, 1);
            throw;
        }

        return boost::to_address(ip);
    }
    void assign(impl_type* p, impl_type const& from) const override
    {
        *p = from;
    }
    void assign(impl_type* p, impl_type&& from) const override
    {
        *p = std::move(from);
    }
};

#endif // IMPL_PTR_DETAIL_DETAIL_HPP
