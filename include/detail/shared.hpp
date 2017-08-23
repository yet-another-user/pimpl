// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_SHARED_HPP
#define IMPL_PTR_DETAIL_SHARED_HPP

#include "./detail.hpp"

namespace detail
{
    template<typename, typename...> struct shared;
}

template<typename impl_type, typename... more_types>
struct detail::shared : std::shared_ptr<impl_type>
{
    using   base_ref = std::shared_ptr<impl_type>&;
    using alloc_type = typename std::conditional<
                       1 <= sizeof...(more_types),
                       typename types<more_types...>::first_type,
                       std::allocator<impl_type>>::type;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        base_ref(*this) = std::allocate_shared<derived_type>(alloc_type(), std::forward<arg_types>(args)...);
    }
};

#endif // IMPL_PTR_DETAIL_SHARED_HPP
