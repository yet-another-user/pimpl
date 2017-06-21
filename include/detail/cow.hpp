#ifndef AUXILIARY_PIMPL_DETAIL_COW_HPP
#define AUXILIARY_PIMPL_DETAIL_COW_HPP

namespace detail
{
    template<typename> struct cow; // copy_on_write
}

template<typename impl_type>
struct detail::cow
{
};

#endif // AUXILIARY_PIMPL_DETAIL_COW_HPP
