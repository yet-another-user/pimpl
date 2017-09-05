// Copyright (c) 2008 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_SHARED_HPP
#define IMPL_PTR_DETAIL_SHARED_HPP

#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct shared;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::shared : std::shared_ptr<impl_type>
{
    using allocator_type = typename std::allocator_traits<allocator>::template rebind_alloc<impl_type>;
    using       base_ref = std::shared_ptr<impl_type>&;

    template<typename derived_type, typename alloc_arg, typename... arg_types>
    void
    emplace(std::allocator_arg_t, alloc_arg&& a0, arg_types&&... args)
    {
        using alloc_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<derived_type>;
        alloc_type a(std::forward<alloc_arg>(a0));
        base_ref(*this) = std::allocate_shared<derived_type>(a, std::forward<arg_types>(args)...);
    }

    shared(std::nullptr_t, const allocator_type&) {}

    template<typename alloc_arg, typename... arg_types>
    shared(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        emplace<impl_type>(std::allocator_arg, std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }
};

#endif // IMPL_PTR_DETAIL_SHARED_HPP
