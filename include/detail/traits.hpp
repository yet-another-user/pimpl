#ifndef IMPL_PTR_DETAIL_TRAITS_HPP
#define IMPL_PTR_DETAIL_TRAITS_HPP

#include <boost/assert.hpp>
#include <boost/utility.hpp>
#include <type_traits>
#include <memory>

namespace detail
{
    // The incomplete-type management technique
    // is originally by Peter Dimov.

    struct traits
    {
        template<typename> struct         base;
        template<typename> struct destroy_type;
        template<typename> struct    copy_base;
        template<typename> struct    copy_type;
    };
}

template<typename impl_type>
struct detail::traits::base
{
    virtual ~base() =default;

    virtual void destroy (impl_type*&) const =0;
};

template<typename impl_type>
struct detail::traits::destroy_type : detail::traits::base<impl_type>
{
    using this_type = detail::traits::destroy_type<impl_type>;
    using base_type = detail::traits::base<impl_type>;

    void destroy (impl_type*& p) const { boost::checked_delete(p); p = 0; }

    operator base_type const*() { static this_type trait; return &trait; }
};

template<typename impl_type>
struct detail::traits::copy_base : detail::traits::base<impl_type>
{
    virtual impl_type* copy (impl_type const*) const =0;
    virtual void     assign (impl_type*&, impl_type const*) const =0;
};

template<typename impl_type>
struct detail::traits::copy_type : detail::traits::copy_base<impl_type>
{
    using this_type = detail::traits::copy_type<impl_type>;
    using base_type = detail::traits::copy_base<impl_type>;

    void    destroy (impl_type*& p) const { boost::checked_delete(p); p = 0; }
    impl_type* copy (impl_type const* p) const { return p ? new impl_type(*p) : nullptr; }

//  void assign (impl_type*& a, impl_type const* b) const
//  {
//      if (a != b) destroy(a), a = copy(b);
//  }
    void assign (impl_type*& a, impl_type const* b) const
    {
        /**/ if (a == b);
        else if (a && b) *a = *b;
        else             destroy(a), a = copy(b);
    }
    operator base_type const*() { static this_type trait; return &trait; }
};

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
