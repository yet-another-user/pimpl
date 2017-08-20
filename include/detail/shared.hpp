#ifndef IMPL_PTR_DETAIL_SHARED_HPP
#define IMPL_PTR_DETAIL_SHARED_HPP

#include <memory>

namespace detail
{
    template<typename, typename =std::allocator<void>> struct shared;
}

template<typename impl_type, typename allocator>
struct detail::shared : std::shared_ptr<impl_type>
{
    using base_ref = std::shared_ptr<impl_type>&;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        using alloc_type = typename allocator::template rebind<derived_type>::other;

        base_ref(*this) = std::allocate_shared<derived_type>(alloc_type(), std::forward<arg_types>(args)...);
    }
};

#endif // IMPL_PTR_DETAIL_SHARED_HPP
