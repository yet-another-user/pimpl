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

#endif // IMPL_PTR_DETAIL_TRAITS_HPP
