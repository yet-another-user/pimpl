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
    static size_t const size_ = 32;
    char storage_[size_];

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        BOOST_ASSERT(size_ <= sizeof(derived_type));

        new (storage_) derived_type(std::forward<arg_types>(args)...);
    }
    onstack () =default;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
