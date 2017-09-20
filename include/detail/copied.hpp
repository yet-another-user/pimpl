// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include <memory>
#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct copied;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::copied
{
    using      this_type = copied;
    using    traits_type = detail::traits::copyable<impl_type, allocator>;
    using       ptr_type = typename traits_type::ptr_type;
    using allocator_type = typename traits_type::alloc_type;

    template<typename derived_type, typename alloc_arg, typename... arg_types>
    void
    emplace(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        impl_ = traits_type::template make<derived_type>(std::allocator_arg, std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }

    template<typename alloc_arg, typename... arg_types>
    copied(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
        : impl_(traits_type::template make<impl_type>(std::allocator_arg, std::forward<alloc_arg>(a), std::forward<arg_types>(args)...))
    {}

    copied (std::nullptr_t, const allocator_type& a) : impl_(nullptr, a) {}
    copied (this_type&& o) = default;
    copied (this_type const& o)
        : impl_(nullptr, o.get_allocator())
    {
        if (o.impl_)
            impl_ = traits_type::make(get_allocator(), *o.impl_);
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        allocator_type a(o.get_allocator());
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_type::assign(impl_.get(), *o.impl_);
        else if ( impl_ && !o.impl_) impl_.reset();
        else if (!impl_ &&  o.impl_) impl_ = traits_type::make(a, *o.impl_);

        return *this;
    }

    void      swap (this_type& o) { std::swap(impl_, o.impl_); }
    impl_type* get () const { return boost::to_address(impl_.get()); }
    long use_count () const { return 1; }

    private:
    allocator_type const& get_allocator() const { return impl_.get_deleter().get_allocator(); }
    allocator_type      & get_allocator()       { return impl_.get_deleter().get_allocator(); }

    ptr_type impl_;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
