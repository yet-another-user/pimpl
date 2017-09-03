#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename =std::allocator<void>> struct copied;
}

template<typename impl_type, typename allocator>
struct impl_ptr_policy::copied
{
    using   this_type = copied;
    using traits_type = detail::traits::copyable<impl_type, allocator>;
    using  traits_ptr = typename traits_type::pointer;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        using   alloc_type = typename std::allocator_traits<allocator>::template rebind_alloc<derived_type>;
        using alloc_traits = std::allocator_traits<alloc_type>;

        alloc_type    a;
        this_type   tmp (nullptr);
        const auto impl (alloc_traits::allocate(a, 1));

        try
        {
            alloc_traits::construct(a, boost::to_address(impl), std::forward<arg_types>(args)...);
            tmp.traits_ = traits_type::singleton();
        }
        catch (...)
        {
            alloc_traits::deallocate(a, impl, 1);
        }

        tmp.impl_   = boost::to_address(impl);

        tmp.swap(*this);
    }

    template<typename... arg_types>
    copied(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

   ~copied () { if (traits_) traits_->destroy(impl_); }
    copied (std::nullptr_t) {}
    copied (this_type&& o) { swap(o); }
    copied (this_type const& o) : traits_(o.traits_)
    {
        if (traits_)
            impl_ = traits_->make(*o.impl_);
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_->assign(impl_, *o.impl_);
        else if ( impl_ && !o.impl_) traits_->destroy(impl_);
        else if (!impl_ &&  o.impl_) impl_ = o.traits_->make(*o.impl_);

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
