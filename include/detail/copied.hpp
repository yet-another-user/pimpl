#ifndef IMPL_PTR_DETAIL_COPIED_HPP
#define IMPL_PTR_DETAIL_COPIED_HPP

#include "./traits.hpp"

namespace detail
{
    template<typename> struct copied;
}

template<typename impl_type>
struct detail::copied
{
    struct traits : detail::traits<traits, impl_type>
    {
        void   destroy (impl_type* p) const override { boost::checked_delete(p); }
        void construct (void*      to, impl_type const& from) const override { ::new(to) impl_type(from); }
        void    assign (impl_type* to, impl_type const& from) const override { *to = from; }

        impl_type* copy_allocate (impl_type const* p) const override { return p ? new impl_type(*p) : nullptr; }
    };

    using this_type = copied;

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        reset(new derived_type(std::forward<arg_types>(args)...));
    }

   ~copied () { if (traits_) traits_->destroy(impl_); }
    copied () {}
    copied (impl_type* p) : impl_(p), traits_(traits()) {}
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

    impl_type*              impl_ = nullptr;
    traits_ptr<impl_type> traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_COPIED_HPP
