#ifndef IMPL_PTR_DETAIL_SHARED_HPP
#define IMPL_PTR_DETAIL_SHARED_HPP

#include "./is_allocator.hpp"
#include <memory>

namespace detail
{
    template<typename> struct shared;
}

template<typename impl_type>
struct detail::shared : std::shared_ptr<impl_type>
{
    using this_type = shared;
    using base_type = std::shared_ptr<impl_type>;
    using  base_ref = base_type&;

    template<typename derived_type, typename... arg_types>
    typename std::enable_if<is_allocator<typename first<arg_types...>::type>::value, void>::type
    emplace(arg_types&&... args)
    {
        base_ref(*this) = std::allocate_shared<derived_type>(std::forward<arg_types>(args)...);
    }
    template<typename derived_type, typename... arg_types>
    typename std::enable_if<!is_allocator<typename first<arg_types...>::type>::value, void>::type
    emplace(arg_types&&... args)
    {
        base_ref(*this) = std::make_shared<derived_type>(std::forward<arg_types>(args)...);
    }
};

#endif // IMPL_PTR_DETAIL_SHARED_HPP
