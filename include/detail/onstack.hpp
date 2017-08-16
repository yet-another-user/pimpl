#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

namespace detail
{
    template<typename, size_t> struct onstack;
}

template<typename impl_type, size_t sz>
struct detail::onstack // Proof of concept
{
    using buffer = char[sz];

    buffer storage_ = {0};
    bool     valid_ = 0;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sz, "");

        new (storage_) derived_type(std::forward<arg_types>(args)...);
        valid_ = true;
    }
    impl_type* get () const { return valid_ ? (impl_type*) storage_ : nullptr; }

    onstack () =default;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
