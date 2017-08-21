// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP
#define IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP

#include <boost/convert/detail/has_member.hpp>

namespace detail
{
    template<typename, typename =void>
    struct is_allocator { BOOST_STATIC_CONSTANT(bool, value = false); };

    template<typename class_type>
    struct is_allocator<class_type, typename std::enable_if<std::is_class<class_type>::value, void>::type>
    {
        BOOST_DECLARE_HAS_MEMBER(has_allocate, allocate);
        BOOST_DECLARE_HAS_MEMBER(has_deallocate, deallocate);

        BOOST_STATIC_CONSTANT(bool, value = has_allocate<class_type>::value && has_deallocate<class_type>::value);
    };
}

#endif // IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP
