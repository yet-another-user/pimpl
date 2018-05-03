// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
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
        template<typename, typename, typename> struct base;

        template<typename, typename> struct   unique;
        template<typename, typename> struct copyable;
    };

    // Helper class to ensure memory gets deallocated regardless of whether construction/destruction throws
    template<typename AT>
    struct dealloc_guard
    {
        using alloc_traits = std::allocator_traits<AT>;
        using      pointer = typename alloc_traits::pointer;
        using   value_type = typename alloc_traits::value_type;

        // take ownership of memory
        constexpr dealloc_guard(AT& a, pointer ptr) noexcept : alloc_(a), ptr_(std::move(ptr)) {}

        dealloc_guard(const dealloc_guard&) = delete;
        dealloc_guard(dealloc_guard&&) = delete;
        dealloc_guard& operator=(const dealloc_guard&) = delete;
        dealloc_guard& operator=(dealloc_guard&&) = delete;

        // give access to raw pointer, e.g. for calling the constructor
        BOOST_CXX14_CONSTEXPR value_type* get() noexcept { return boost::to_address(ptr_); }
        // transfer ownership to caller
        BOOST_CXX14_CONSTEXPR pointer release() noexcept { pointer r = ptr_; ptr_ = nullptr; return r; }

        ~dealloc_guard()
        {
            if (ptr_)
                alloc_traits::deallocate(alloc_, ptr_, 1);
        }

        private:
        AT& alloc_;
        pointer ptr_;
    };
}

template<typename traits_type, typename impl_type, typename AT>
struct detail::traits::base
{
    using    this_type = base<traits_type, impl_type, AT>;
    using   alloc_type = typename std::allocator_traits<AT>::template rebind_alloc<impl_type>;
    using alloc_traits = std::allocator_traits<alloc_type>;
    using      pointer = typename alloc_traits::pointer;
    struct deleter;
    using     ptr_type = std::unique_ptr<impl_type, deleter>;

    struct deleter : private alloc_type
    {
        // type used by unique_ptr as replacement for T*
        using        pointer = base::pointer;
        using allocator_type = alloc_type;

        void operator()(pointer ip) { base::destroy(get_allocator(), ip); }

        constexpr deleter(const allocator_type& a) : allocator_type(a) {}
        constexpr             allocator_type const& get_allocator() const { return *this; }
        BOOST_CXX14_CONSTEXPR allocator_type      & get_allocator()       { return *this; }
    };

    virtual ~base() =default;

    template<typename derived_type, typename... arg_types>
    static void emplace(
            typename alloc_traits::
                template rebind_alloc<
                    typename std::remove_cv<derived_type>::type
            >& alloc
          , derived_type* p
          , arg_types&&... args)
    {
        using alloc_traits = typename alloc_traits::template rebind_traits<typename std::remove_cv<derived_type>::type>;

        construct_singleton();
        alloc_traits::construct(alloc, p, std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename allocator, typename... arg_types>
    static ptr_type make(std::allocator_arg_t, allocator&& a0, arg_types&&... args)
    {
        using    alloc_type = typename std::allocator_traits<typename std::decay<allocator>::type>::template rebind_alloc<derived_type>;
        using  alloc_traits = std::allocator_traits<alloc_type>;
        using dealloc_guard = detail::dealloc_guard<alloc_type>;

        alloc_type     a(std::forward<allocator>(a0));
        dealloc_guard ap(a, alloc_traits::allocate(a, 1));

        emplace(a, ap.get(), std::forward<arg_types>(args)...);
        return ptr_type(ap.release(), std::move(a));
    }

    static void       destroy (alloc_type& a, pointer p                       ) { return traits_->do_destroy  (a, p                 ); }
    static void        assign (               pointer p, impl_type const& from) { return traits_->do_assign   (   p,           from ); }
    static void        assign (               pointer p, impl_type     && from) { return traits_->do_assign   (   p, std::move(from)); }
    static void     construct (alloc_type& a, void*   p, impl_type const& from) { return traits_->do_construct(a, p,           from ); }
    static void     construct (alloc_type& a, void*   p, impl_type     && from) { return traits_->do_construct(a, p, std::move(from)); }
    static ptr_type      make (alloc_type& a,            impl_type const& from) { return traits_->do_make     (a,              from ); }
    static ptr_type      make (alloc_type& a,            impl_type     && from) { return traits_->do_make     (a,    std::move(from)); }

    protected:

    void destroy_(alloc_type& a, pointer p) const
    {
        dealloc_guard<alloc_type> ap(a, std::move(p));
        alloc_traits::destroy(a, ap.get());
    }

    private:

    virtual void       do_destroy (alloc_type&, pointer) const =0;
    virtual void        do_assign (pointer, impl_type const&) const =0;
    virtual void        do_assign (pointer, impl_type&&) const =0;
    virtual void     do_construct (alloc_type&, void*, impl_type const&) const =0;
    virtual void     do_construct (alloc_type&, void*, impl_type&& ) const =0;
    virtual ptr_type      do_make (alloc_type&, impl_type const&) const =0;
    virtual ptr_type      do_make (alloc_type&, impl_type&& ) const =0;

    static void construct_singleton()
    {
        static_assert(!std::is_same<this_type, traits_type>::value, "");
        static_assert(std::is_base_of<this_type, traits_type>::value, "");

        static traits_type const traits = ((traits_ = &traits), traits_type{});
    }
    private: static base const* traits_;
};

template<typename traits_type, typename impl_type, typename alloc_type>
detail::traits::base<traits_type, impl_type, alloc_type> const*
detail::traits::base<traits_type, impl_type, alloc_type>::traits_;

template<typename impl_type, typename allocator>
struct detail::traits::unique final : base<unique<impl_type, allocator>, impl_type, allocator>
{
    using  this_type = unique<impl_type, allocator>;
    using  base_type = base<this_type, impl_type, allocator>;
    using alloc_type = typename base_type::alloc_type;
    using    pointer = typename base_type::pointer;
    using   ptr_type = typename base_type::ptr_type;

    void       do_destroy (alloc_type& a, pointer p                  ) const override { this->destroy_(a, p); }
    void        do_assign (               pointer  , impl_type const&) const override { BOOST_ASSERT(!"not implemented"); }
    void        do_assign (               pointer  , impl_type&&     ) const override { BOOST_ASSERT(!"not implemented"); }
    void     do_construct (alloc_type&  , void*    , impl_type const&) const override { BOOST_ASSERT(!"not implemented"); }
    void     do_construct (alloc_type&  , void*    , impl_type&&     ) const override { BOOST_ASSERT(!"not implemented"); }
    ptr_type      do_make (alloc_type& a,            impl_type const&) const override { BOOST_ASSERT(!"not implemented"); return ptr_type(nullptr, a); }
    ptr_type      do_make (alloc_type& a,            impl_type&&     ) const override { BOOST_ASSERT(!"not implemented"); return ptr_type(nullptr, a); }
};

template<typename impl_type, typename allocator>
struct detail::traits::copyable final : base<copyable<impl_type, allocator>, impl_type, allocator>
{
    using    this_type = copyable<impl_type, allocator>;
    using    base_type = base<this_type, impl_type, allocator>;
    using   alloc_type = typename base_type::alloc_type;
    using alloc_traits = typename base_type::alloc_traits;
    using      pointer = typename base_type::pointer;
    using     ptr_type = typename base_type::ptr_type;

    void do_destroy(alloc_type& a, pointer p) const override { this->destroy_(a, p); }

    void
    do_construct(alloc_type& a, void* vp, impl_type const& from) const override
    {
        this->emplace(a, static_cast<impl_type*>(vp), from);
    }
    void
    do_construct(alloc_type& a, void* vp, impl_type&& from) const override
    {
        this->emplace(a, static_cast<impl_type*>(vp), std::move(from));
    }
    ptr_type
    do_make(alloc_type& a, impl_type const& from) const override
    {
        return this->template make<impl_type>(std::allocator_arg, a, from);
    }
    ptr_type
    do_make(alloc_type& a, impl_type&& from) const override
    {
        return this->template make<impl_type>(std::allocator_arg, a, std::move(from));
    }
    void
    do_assign(pointer p, impl_type const& from) const override
    {
        *p = from;
    }
    void
    do_assign(pointer p, impl_type&& from) const override
    {
        *p = std::move(from);
    }
};

#endif // IMPL_PTR_DETAIL_DETAIL_HPP
