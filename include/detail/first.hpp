// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_FIRST_HPP
#define IMPL_PTR_DETAIL_FIRST_HPP

namespace detail
{
    template <typename first_type =void, typename...> struct first { using type = first_type; };

//    using alloc = typename std::conditional<
//                  sizeof...(arg_types) == 0,
//                  std::allocator<impl_type>,
//                  typename first<arg_types...>::type>::type;
};

#endif // IMPL_PTR_DETAIL_FIRST_HPP
