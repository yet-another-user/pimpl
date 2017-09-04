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

    struct deleter
    {
        void operator()(impl_type* ip) const { traits().destroy(ip); }
    };

    virtual ~base() =default;

    virtual void         destroy (impl_type*) const =0;
    virtual void          assign (impl_type*, impl_type const&) const { BOOST_ASSERT(!"not implemented"); }
    virtual void          assign (impl_type* p, impl_type&& from) const { assign(p, from); }
    virtual impl_type* construct (void*, impl_type const&) const { BOOST_ASSERT(!"not implemented"); return nullptr; }
    virtual impl_type* construct (void* p, impl_type&& from) const { return construct(p, from); }

    template<typename alloctor, typename... arg_types>
    static void emplace(alloctor&& a
            , typename std::allocator_traits<typename std::decay<alloctor>::type>::pointer p
            , arg_types&&... args)
    {
        using alloc_traits = std::allocator_traits<typename std::decay<alloctor>::type>;

        construct_singleton();
        alloc_traits::construct(a, boost::to_address(p), std::forward<arg_types>(args)...);
    }

    static const base& traits()
    {
        return *traits_;
    }
    private:
    static void construct_singleton()
    {
        static_assert(!std::is_same<this_type, traits_type>::value, "");
        static_assert(std::is_base_of<this_type, traits_type>::value, "");

        static traits_type const traits = ((traits_ = &traits), traits_type{});
    }
    private: static pointer traits_;
};

template<typename traits_type, typename impl_type>
typename detail::traits::base<traits_type, impl_type>::pointer
detail::traits::base<traits_type, impl_type>::traits_;

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

    void
    destroy(impl_type* p) const override
    {
        alloc_type a;

        alloc_traits::destroy(a, p);
        alloc_traits::deallocate(a, p, 1);
    }
    impl_type*
    construct(void* vp, impl_type const& from) const override
    {
        alloc_type  a;
        impl_type* ap = vp ? nullptr : alloc_traits::allocate(a, 1);
        impl_type* ip = vp ? static_cast<impl_type*>(vp) : boost::to_address(ap);

        try
        {
            alloc_traits::construct(a, ip, from);
        }
        catch (...)
        {
            if (ap) alloc_traits::deallocate(a, ap, 1);
            throw;
        }
        return ip;
    }
    impl_type*
    construct(void* vp, impl_type&& from) const override
    {
        alloc_type  a;
        impl_type* ap = vp ? nullptr : alloc_traits::allocate(a, 1);
        impl_type* ip = vp ? static_cast<impl_type*>(vp) : boost::to_address(ap);

        try
        {
            alloc_traits::construct(a, ip, std::move(from));
        }
        catch (...)
        {
            if (ap) alloc_traits::deallocate(a, ap, 1);
            throw;
        }
        return ip;
    }
    void
    assign(impl_type* p, impl_type const& from) const override
    {
        *p = from;
    }
    void
    assign(impl_type* p, impl_type&& from) const override
    {
        *p = std::move(from);
    }
};

#endif // IMPL_PTR_DETAIL_DETAIL_HPP
