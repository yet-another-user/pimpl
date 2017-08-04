#ifndef AUXILIARY_PIMPL_DETAIL_UNIQUE_HPP
#define AUXILIARY_PIMPL_DETAIL_UNIQUE_HPP

#include <boost/assert.hpp>
#include <boost/utility.hpp>
#include <type_traits>
#include <memory>
#include "./is_allocator.hpp"

namespace detail
{
    template<typename> struct unique;

    struct traits
    {
        template<typename> struct      base;
        template<typename> struct deep_copy;
    };
}

template<typename impl_type>
struct detail::traits::base
{
    virtual ~base() =default;

    virtual void    destroy (impl_type*&) const =0;
    virtual impl_type* copy (impl_type const*) const =0;
    virtual void     assign (impl_type*&, impl_type const*) const =0;
};

template<typename impl_type>
struct detail::traits::deep_copy : detail::traits::base<impl_type>
{
    using this_type = detail::traits::deep_copy<impl_type>;
    using base_type = detail::traits::base<impl_type>;

    void    destroy (impl_type*& p) const { boost::checked_delete(p); p = 0; }
    impl_type* copy (impl_type const* p) const { return p ? new impl_type(*p) : 0; }

    void assign (impl_type*& a, impl_type const* b) const
    {
        if (a != b) destroy(a), a = copy(b);
    }
    // TODO: add selection using is_copy_assignable
//        void assign (impl_type*& a, impl_type const* b) const
//        {
//            /**/ if ( a ==  b);
//            else if ( a &&  b) *a = *b;
//            else if (!a &&  b) a = copy(b);
//            else if ( a && !b) destroy(a);
//        }
    operator base_type const*() { static this_type trait; return &trait; }
};

template<typename impl_type>
struct detail::unique
{
    // Smart-pointer with the value-semantics behavior.
    // The incomplete-type management technique is originally by Peter Dimov.

    using        this_type = unique;
    using base_traits_type = detail::traits::base<impl_type>;
    using copy_traits_type = detail::traits::deep_copy<impl_type>;

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

   ~unique () { if (traits_) traits_->destroy(impl_); }
    unique () {}
    unique (impl_type* p) : impl_(p), traits_(copy_traits_type()) {}
    unique (this_type&& o) { swap(o); }
    unique (this_type const& o)
    :
        impl_(o.traits_ ? o.traits_->copy(o.impl_) : nullptr),
        traits_(o.traits_)
    {}

    this_type& operator= (this_type&& o) { swap(o); return *this; }
    this_type& operator= (this_type const& o) { traits_ = o.traits_; traits_->assign(impl_, o.impl_); return *this; }
    bool       operator< (this_type const& o) const { return impl_ < o.impl_; }

    void     reset (impl_type* p) { this_type(p).swap(*this); }
    void      swap (this_type& o) { std::swap(impl_, o.impl_), std::swap(traits_, o.traits_); }
    impl_type* get () const { return impl_; }
    long use_count () const { return 1; }

    private:

    impl_type*                impl_ = nullptr;
    base_traits_type const* traits_ = nullptr;
};

#endif // AUXILIARY_PIMPL_DETAIL_UNIQUE_HPP
