// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
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
    using    del_type = typename traits_type::deleter;
    using    ptr_type = std::unique_ptr<impl_type, del_type>;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        impl_.reset(traits_type::template make<derived_type>(std::forward<arg_types>(args)...));
    }

    template<typename... arg_types>
    unique(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

   ~unique () = default;
    unique (std::nullptr_t) {}

    unique (this_type&& o) = default;
    this_type& operator= (this_type&& o) { swap(o); return *this; }

    unique (this_type const&) =delete;
    this_type& operator= (this_type const&) =delete;

    bool operator< (this_type const& o) const { return impl_ < o.impl_; }
    void      swap (this_type& o) { std::swap(impl_, o.impl_); }
    impl_type* get () const { return const_cast<impl_type*>(boost::to_address(impl_.get())); }
    long use_count () const { return 1; }

    private: ptr_type impl_;
};

#endif // IMPL_PTR_DETAIL_UNIQUE_HPP
