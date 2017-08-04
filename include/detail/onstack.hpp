#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

namespace detail
{
    template<typename> struct onstack;
}

template<typename impl_type>
struct detail::onstack
{
    // Proof of concept
    // Need to extract storage size from more_types
    char storage_[32];

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        new (storage_) derived_type(std::forward<arg_types>(args)...);
    }
    onstack () =default;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
