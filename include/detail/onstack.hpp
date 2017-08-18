#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

#include "./traits.hpp"
#include <boost/type_traits/aligned_storage.hpp>

namespace detail
{
    template<typename, size_t> struct onstack;
}

template<typename impl_type, size_t sz>
struct detail::onstack // Proof of concept
{
    using        this_type = onstack;
    using real_traits_type = detail::traits::onstack_type<impl_type>;
    using base_traits_type = detail::traits::base<impl_type>;
    using     storage_type = typename boost::aligned_storage<sz, 8>::type;

   ~onstack () { if (traits_) traits_->destroy(get()); }
    onstack () {}
    onstack (this_type const& o)
    :
        traits_(o.traits_)
    {
        if (traits_)
            traits_->construct(get(), o.get());
    }
    this_type& operator=(this_type const& o)
    {
        if (traits_)
            traits_->destroy(get());

        traits_ = o.traits_;

        if (traits_)
            traits_->construct(get(), o.get());

        return *this;
    }

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sz, "");

        ::new (storage_.address()) derived_type(std::forward<arg_types>(args)...);
        traits_ = real_traits_type();
    }
    impl_type* get () const { return traits_ ? (impl_type*) storage_.address() : nullptr; }

    private:

    storage_type           storage_;
    base_traits_type const* traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
