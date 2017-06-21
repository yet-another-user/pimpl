#ifndef AUXILIARY_PIMPL_DETAIL_ONSTACK_HPP
#define AUXILIARY_PIMPL_DETAIL_ONSTACK_HPP

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

    template<typename... arg_types>
    void
    construct(arg_types&&... args)
    {
        new (storage_) impl_type(std::forward<arg_types>(args)...);
    }
    onstack () =default;
};

#endif // AUXILIARY_PIMPL_DETAIL_ONSTACK_HPP
