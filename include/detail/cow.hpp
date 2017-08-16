#ifndef IMPL_PTR_DETAIL_COW_HPP
#define IMPL_PTR_DETAIL_COW_HPP

#include "./traits.hpp"
#include "./is_allocator.hpp"

namespace detail
{
    template<typename> struct cow;
}

template<typename impl_type>
struct detail::cow
{
};

#endif // IMPL_PTR_DETAIL_COW_HPP
