// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

#include "./optional.hpp"

namespace detail
{
    template<typename, typename =void> struct onstack;
}

template<typename impl_type, typename size_type>
struct detail::onstack : detail::optional<impl_type, size_type> // Proof of concept
{
    // Inherit all constructors (needed for in_place)
    using detail::optional<impl_type, size_type>::optional;

    // Don't permit default construction (only way to get a null-state)
    onstack () = delete;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
