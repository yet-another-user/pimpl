#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include "./traits.hpp"

namespace detail
{
    template<typename, typename> struct copied;
}

template<typename impl_type, typename allocator>
struct detail::copied
{
    template<typename T> using alloc = typename allocator::template rebind<T>::other;

    using   this_type = copied;
    using traits_type = detail::copyable_traits<impl_type, allocator>;
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

   ~copied () { if (traits_) traits_->destroy(impl_); }
    copied () {}
    copied (impl_type* p) : impl_(p), traits_(traits_type()) {}
    copied (this_type&& o) { swap(o); }
    copied (this_type const& o) : traits_(o.traits_)
    {
        if (traits_)
            impl_ = traits_->construct(nullptr, *o.impl_);
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_->assign(impl_, *o.impl_);
        else if ( impl_ && !o.impl_) traits_->destroy(impl_);
        else if (!impl_ &&  o.impl_) impl_ = o.traits_->construct(nullptr, *o.impl_);

        traits_ = o.traits_;

        return *this;
    }

    void      swap (this_type& o) { std::swap(impl_, o.impl_), std::swap(traits_, o.traits_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*   impl_ = nullptr;
    traits_ptr traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
