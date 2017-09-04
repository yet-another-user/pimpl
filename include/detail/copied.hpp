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

        alloc_type     a;
        derived_type* ap = alloc_traits::allocate(a, 1);
        derived_type* ip = boost::to_address(ap);

        try
        {
            traits_type::emplace(a, ip, std::forward<arg_types>(args)...);

            this_type(ip).swap(*this);
        }
        catch (...)
        {
            alloc_traits::deallocate(a, ap, 1);
            throw;
        }
    }

    template<typename... arg_types>
    copied(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

   ~copied () { if (impl_) traits_type::traits().destroy(impl_); }
    copied (std::nullptr_t) {}
    copied (impl_type* p) : impl_(p) {}
    copied (this_type&& o) { swap(o); }
    copied (this_type const& o)
    {
        if (o.impl_)
            impl_ = traits_type::traits().construct(nullptr, *o.impl_);
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_type::traits().assign(impl_, *o.impl_);
        else if ( impl_ && !o.impl_) { traits_type::traits().destroy(impl_); impl_ = nullptr; }
        else if (!impl_ &&  o.impl_) impl_ = traits_type::traits().construct(nullptr, *o.impl_);

        return *this;
    }

    void      swap (this_type& o) { std::swap(impl_, o.impl_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*   impl_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
