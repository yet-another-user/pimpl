#ifndef IMPL_PTR_DETAIL_UNIQUE_HPP
#define IMPL_PTR_DETAIL_UNIQUE_HPP

#include "./traits.hpp"

namespace detail
{
    template<typename, typename> struct unique;
}

template<typename impl_type, typename allocator>
struct detail::unique
{
    template<typename T> using alloc = typename allocator::template rebind<T>::other;

    using   this_type = unique;
    using traits_type = detail::unique_traits<impl_type, allocator>;
    using  traits_ptr = typename traits_type::pointer;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        alloc<derived_type> a;
        impl_type*       impl = a.allocate(1);

        a.construct(impl, std::forward<arg_types>(args)...);

        this_type(impl).swap(*this);
    }

   ~unique () { if (traits_) traits_->destroy(impl_); }
    unique () {}
    unique (impl_type* p) : impl_(p), traits_(traits_type()) {}

    unique (this_type&& o) { swap(o); }
    this_type& operator= (this_type&& o) { swap(o); return *this; }

    unique (this_type const&) =delete;
    this_type& operator= (this_type const&) =delete;

    bool operator< (this_type const& o) const { return impl_ < o.impl_; }
    void      swap (this_type& o) { std::swap(impl_, o.impl_), std::swap(traits_, o.traits_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*   impl_ = nullptr;
    traits_ptr traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_UNIQUE_HPP
