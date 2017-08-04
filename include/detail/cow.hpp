#ifndef IMPL_PTR_DETAIL_COW_HPP
#define IMPL_PTR_DETAIL_COW_HPP

namespace detail
{
    template<typename> struct cow; // copy_on_write
}

template<typename impl_type>
struct detail::cow
{
};

#endif // IMPL_PTR_DETAIL_COW_HPP
