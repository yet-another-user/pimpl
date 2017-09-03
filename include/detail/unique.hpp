// Copyright (c) 2008 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_UNIQUE_HPP
#define IMPL_PTR_DETAIL_UNIQUE_HPP

#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct unique;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::unique
{
    using   this_type = unique;
    using traits_type = detail::traits::unique<impl_type, allocator>;
    using  traits_ptr = typename traits_type::pointer;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        using   alloc_type = typename std::allocator_traits<allocator>::template rebind_alloc<derived_type>;
        using alloc_traits = std::allocator_traits<alloc_type>;

        alloc_type    a;
        this_type   tmp (nullptr);
        const auto impl (alloc_traits::allocate(a, 1));

        try
        {
            alloc_traits::construct(a, boost::to_address(impl), std::forward<arg_types>(args)...);
            tmp.traits_ = traits_type::singleton();
        }
        catch (...)
        {
            alloc_traits::deallocate(a, impl, 1);
            throw;
        }

        tmp.impl_   = boost::to_address(impl);

        tmp.swap(*this);
    }

    template<typename... arg_types>
    unique(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

   ~unique () { if (traits_) traits_->destroy(impl_); }
    unique (std::nullptr_t) {}

    unique (this_type&& o) { swap(o); }
    this_type& operator= (this_type&& o) { swap(o); return *this; }

    unique (this_type const&) =delete;
    this_type& operator= (this_type const&) =delete;

    bool operator< (this_type const& o) const { return impl_ < o.impl_; }
    void      swap (this_type& o) { std::swap(impl_, o.impl_), std::swap(traits_, o.traits_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*   impl_ = nullptr;
    traits_ptr traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_UNIQUE_HPP
