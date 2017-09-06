// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct copied;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::copied
{
    using   this_type = copied;
    using traits_type = detail::traits::copyable<impl_type, allocator>;
    using     pointer = std::unique_ptr<impl_type, typename traits_type::deleter>;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        using   alloc_type = typename std::allocator_traits<allocator>::template rebind_alloc<derived_type>;
        using alloc_traits = std::allocator_traits<alloc_type>;

        alloc_type a;
        auto      ap = alloc_traits::allocate(a, 1);

        try
        {
            traits_type::emplace(a, boost::to_address(ap), std::forward<arg_types>(args)...);
            impl_.reset(ap);
        }
        catch (...)
        {
            alloc_traits::deallocate(a, ap, 1);
            throw;
        }
    }

    template<typename... arg_types>
    copied(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

    copied (std::nullptr_t) {}
    copied (this_type&& o) = default;
    copied (this_type const& o)
    {
        if (o.impl_)
            impl_.reset(traits_type::construct(nullptr, *o.impl_));
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_type::assign(impl_.get(), *o.impl_);
        else if ( impl_ && !o.impl_) impl_.reset();
        else if (!impl_ &&  o.impl_) impl_.reset(traits_type::construct(nullptr, *o.impl_));

        return *this;
    }

    void      swap (this_type& o) { std::swap(impl_, o.impl_); }
    impl_type* get () const { return impl_.get(); }
    long use_count () const { return 1; }

    private: pointer impl_;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
