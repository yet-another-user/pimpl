#ifndef IMPL_PTR_DETAIL_COW_HPP
#define IMPL_PTR_DETAIL_COW_HPP

#include "./traits.hpp"

namespace detail
{
    template<typename, typename =std::allocator<void>> struct cow;
}

template<typename impl_type, typename allocator>
struct detail::cow
{
};

#endif // IMPL_PTR_DETAIL_COW_HPP
