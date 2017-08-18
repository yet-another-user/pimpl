#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include "./traits.hpp"
#include "./is_allocator.hpp"

namespace detail
{
    template<typename> struct copied;
}

template<typename impl_type>
struct detail::copied
{
    // Smart-pointer with the value-semantics behavior.

    using   this_type = copied;
    using traits_type = traits::copyable<impl_type>;

//  template<typename derived_type, typename alloc_type, typename... arg_types>
//  typename std::enable_if<is_allocator<alloc_type>::value, void>::type
//  emplace(alloc_type&& alloc, arg_types&&... args)
//  {
//      void* mem = std::allocator_traits<alloc_type>::allocate(alloc, 1);
//      reset(new(mem) derived_type(std::forward<arg_types>(args)...));
//  }
    template<typename derived_type, typename... arg_types>
    typename std::enable_if<!is_allocator<typename first<arg_types...>::type>::value, void>::type
    emplace(arg_types&&... args)
    {
        reset(new derived_type(std::forward<arg_types>(args)...));
    }

   ~copied () { if (traits_) traits_->destroy(impl_); }
    copied () {}
    copied (impl_type* p) : impl_(p), traits_(traits_type()) {}
    copied (this_type&& o) { swap(o); }
    copied (this_type const& o) : traits_(o.traits_)
    {
        if (traits_)
            impl_ = traits_->copy_allocate(o.impl_);
    }

    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }
    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o)
    {
        /**/ if ( impl_ ==  o.impl_);
        else if ( impl_ &&  o.impl_) traits_->assign(impl_, *o.impl_);
        else if ( impl_ && !o.impl_) traits_->destroy(impl_);
        else if (!impl_ &&  o.impl_) impl_ = o.traits_->copy_allocate(o.impl_);

        traits_ = o.traits_;

        return *this;
    }

    void     reset (impl_type* p) { this_type(p).swap(*this); }
    void      swap (this_type& o) { std::swap(impl_, o.impl_), std::swap(traits_, o.traits_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*                   impl_ = nullptr;
    traits::pointer<impl_type> traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
