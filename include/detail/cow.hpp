#ifndef IMPL_PTR_DETAIL_COW_HPP
#define IMPL_PTR_DETAIL_COW_HPP

#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct cow;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::cow
{
};

#endif // IMPL_PTR_DETAIL_COW_HPP
