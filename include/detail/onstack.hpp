#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

namespace detail
{
    template<typename, size_t> struct onstack;
}

template<typename impl_type, size_t sz>
struct detail::onstack
{
    // Proof of concept
    char storage_[sz];

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        BOOST_ASSERT(sizeof(derived_type) <= sz);

        new (storage_) derived_type(std::forward<arg_types>(args)...);
    }
    impl_type* get () const { return (impl_type*) storage_; }

    onstack () =default;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
