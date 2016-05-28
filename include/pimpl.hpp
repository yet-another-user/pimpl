// Copyright (c) 2006-2016 Mike Wazowski.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef AUXILIARY_PIMPL_HPP
#define AUXILIARY_PIMPL_HPP

#include <boost/assert.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <type_traits>
#include <memory>

template<class user_type>
struct pimpl
{
    static user_type null();

    struct                      implementation;
    template<class> class            value_ptr;
    template<template<class> class> class base;

    using   unique = base<value_ptr>;
    using   shared = base<std::shared_ptr>;
    using yes_type = boost::type_traits::yes_type;
    using  no_type = boost::type_traits::no_type;
    using ptr_type = typename std::remove_reference<user_type>::type*;

    template<class Y> static yes_type test (Y const*, typename Y::pimpl_type const* =0);
    /***************/ static no_type  test (...);

    BOOST_STATIC_CONSTANT(bool, value = (1 == sizeof(test(ptr_type(0)))));
};

template<class user_type>
template<class impl_type>
struct pimpl<user_type>::value_ptr
{
    // Smart-pointer with the value-semantics behavior.
    // The incomplete-type management technique is originally by Peter Dimov.

   ~value_ptr () { traits_->destroy(impl_); }
    value_ptr () : traits_(traits()), impl_(0) {}
    value_ptr (impl_type* p) : traits_(deep_copy()), impl_(p) {}
    value_ptr (value_ptr const& that) : traits_(that.traits_), impl_(traits_->copy(that.impl_)) {}

    value_ptr& operator= (value_ptr const& that) { traits_ = that.traits_; traits_->assign(impl_, that.impl_); return *this; }
    bool       operator< (value_ptr const& that) const { return this->impl_ < that.impl_; }

    void           reset (impl_type* p) { value_ptr(p).swap(*this); }
    void            swap (value_ptr& that) { std::swap(impl_, that.impl_), std::swap(traits_, that.traits_); }
    impl_type*       get () { return impl_; }
    impl_type const* get () const { return impl_; }
    long       use_count () const { return 1; }

    private:

    struct traits
    {
        virtual ~traits() =default;

        virtual void    destroy (impl_type*&) const {}
        virtual impl_type* copy (impl_type const*) const { return nullptr; }
        virtual void     assign (impl_type*&, impl_type const*) const {}

        operator traits const*() { static traits impl; return &impl; }
    };
    struct deep_copy : public traits
    {
        virtual void    destroy (impl_type*& p) const { boost::checked_delete(p); p = 0; }
        virtual impl_type* copy (impl_type const* p) const { return p ? new impl_type(*p) : 0; }
        virtual void     assign (impl_type*& a, impl_type const* b) const
        {
            /**/ if ( a ==  b);
            else if ( a &&  b) *a = *b;
            else if (!a &&  b) a = copy(b);
            else if ( a && !b) destroy(a);
        }
        operator traits const*() { static deep_copy impl; return &impl; }
    };

    traits const* traits_;
    impl_type*      impl_;
};

template<class user_type>
template<template<class> class manager>
struct pimpl<user_type>::base
{
    struct null_type {};

    using implementation = typename pimpl<user_type>::implementation;
    using     pimpl_type = base;
    using   managed_type = manager<implementation>;

    template<class T> using     rm_ref = typename std::remove_reference<T>::type;
    template<class T> using is_derived = typename std::enable_if<std::is_base_of<base, rm_ref<T>>::value, null_type*>::type;

    bool         operator! () const { return !impl_.get(); }
    explicit operator bool () const { return  impl_.get(); }

    // Comparison Operators.
    // base::op==() transfers the comparison to 'impl_' (std::shared_ptr or pimpl::value_ptr).
    // Consequently, pointer-semantics (shared_ptr-based) pimpls are comparable as there is shared_ptr::op==().
    // However, value-semantics (value_ptr-based) pimpls are NOT COMPARABLE BY DEFAULT -- the standard
    // value-semantics behavior -- as there is no pimpl::value_ptr::op==().
    // If a value-semantics class T needs to be comparable, then it has to provide
    // T::op==(T const&) EXPLICITLY as part of its public interface.
    // Trying to call this base::op==() for value_ptr-based pimpl will fail to compile (no value_ptr::op==())
    // and will indicate that the user forgot to declare T::operator==(T const&).
    bool operator==(pimpl_type const& that) const { return impl_ == that.impl_; }
    bool operator!=(pimpl_type const& that) const { return impl_ != that.impl_; }
    bool operator< (pimpl_type const& that) const { return impl_  < that.impl_; }

    void swap (pimpl_type& that) { impl_.swap(that.impl_); }
    void swap (managed_type& that) { impl_.swap(that); }

    void reset(implementation* p) { impl_.reset(p); }
    template<class Y, class D> void reset(Y* p, D d) { impl_.reset(p, d); }

    // Access To the Implementation.
    // Functions allow access to the underlying implementation. These methods
    // are only meaningful in the code for which pimpl<>::implementation has been
    // made visible. Pimpl behaves like a proxy and exhibits the deep-constness
    // property unlike raw pointers and, say, std::shared_ptr. That is,
    // 'const std::shared_ptr' still allows the underlying data modified!.
    // Pimpl does not. So, 'const pimpls' return 'const' pointers and references.
    implementation const* operator->() const { BOOST_ASSERT(impl_.get()); return  impl_.get(); }
    implementation const& operator *() const { BOOST_ASSERT(impl_.get()); return *impl_.get(); }
    implementation*       operator->()       { BOOST_ASSERT(impl_.get()); return  impl_.get(); }
    implementation&       operator *()       { BOOST_ASSERT(impl_.get()); return *impl_.get(); }

    long use_count() const { return impl_.use_count(); }

    protected:

    template<typename> friend class pimpl;

    base (null_type) {}
    base () : impl_(new implementation()) {}

    template<class Arg>
    base(Arg&& arg, is_derived<Arg> =0) : impl_(arg.impl_) {}

    template<typename... Args>
    base(Args&&... args) : impl_(new implementation(std::forward<Args>(args)...)) {}

    private: managed_type impl_;
};

template<class user_type>
inline
user_type
pimpl<user_type>::null()
{
    // null_type needs to be declared in pimpl::base.
    // That way null_type below easily matches pimpl_type and,
    // therefore, base::base(null_type) is called correctly.

    using  null_type = typename user_type::null_type;
    using pimpl_type = typename user_type::pimpl_type;

    static_assert(pimpl<user_type>::value, "");
    static_assert(sizeof(user_type) == sizeof(pimpl_type), "");

    null_type   arg;
    pimpl_type null (arg);

    return *(user_type*) &null;
}

#endif // AUXILIARY_PIMPL_HPP
