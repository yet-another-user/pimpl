// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_UNIQUE_HPP
#define IMPL_PTR_DETAIL_UNIQUE_HPP

#include <memory>
#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct unique;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::unique
{
    using      this_type = unique;
    using    traits_type = detail::traits::unique<impl_type, allocator>;
    using       ptr_type = typename traits_type::ptr_type;
    using allocator_type = typename traits_type::alloc_type;

    template<typename derived_type, typename alloc_arg, typename... arg_types>
    void
    emplace(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        impl_ = traits_type::template make<derived_type>(std::allocator_arg, std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }

    template<typename alloc_arg, typename... arg_types>
    unique(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        emplace<impl_type>(std::allocator_arg, std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }

    unique (std::nullptr_t) {}

    unique (this_type&& o) = default;
    this_type& operator= (this_type&& o) { swap(o); return *this; }

    unique (this_type const&) =delete;
    this_type& operator= (this_type const&) =delete;

    bool operator< (this_type const& o) const { return impl_ < o.impl_; }
    void      swap (this_type& o) { std::swap(impl_, o.impl_); }
    impl_type* get () const { return boost::to_address(impl_.get()); }
    long use_count () const { return 1; }

    private: ptr_type impl_;
};

#endif // IMPL_PTR_DETAIL_UNIQUE_HPP
