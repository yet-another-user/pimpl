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
        BOOST_ASSERT(sizeof(derived_type) <= size_);

        new (storage_) derived_type(std::forward<arg_types>(args)...);
    }
    impl_type* get () const { return (impl_type*) storage_; }

    onstack () =default;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
