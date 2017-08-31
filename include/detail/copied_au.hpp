#ifndef IMPL_PTR_DETAIL_COPIED_AU_HPP
#define IMPL_PTR_DETAIL_COPIED_AU_HPP

// Andrey. Please add you copyright here.

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct copied_au;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::copied_au
{
};

#endif // IMPL_PTR_DETAIL_COPIED_AU_HPP
