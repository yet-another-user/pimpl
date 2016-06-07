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

namespace pimpl_detail
{
    template <typename first_type =void, typename...>
    struct first
    {
       using type = first_type;
    };
    template<typename, typename...> struct shared;
    template<typename, typename...> struct unique;
    template<typename, typename...> struct onstack;
}

template<typename impl_type, typename... more_types>
struct pimpl_detail::onstack
{
    // Proof of concept
    // Need to extract storage size from more_types
    char storage_[32];

    template<typename... arg_types>
    void
    make(arg_types&&... args)
    {
        new (storage_) impl_type(std::forward<arg_types>(args)...);
    }

    onstack () =default;
};

template<typename impl_type, typename... more_types>
struct pimpl_detail::shared : public std::shared_ptr<impl_type>
{
    using this_type = shared;
    using base_type = std::shared_ptr<impl_type>;
    using     alloc = typename std::conditional<
                      sizeof...(more_types) == 0,
                      std::allocator<impl_type>,
                      typename first<more_types...>::type>::type;

    template<typename... arg_types>
    void
    make(arg_types&&... args)
    {
        base_type bt = std::allocate_shared<impl_type>(alloc(), std::forward<arg_types>(args)...);
        this->swap(bt);
    }
};

template<typename impl_type, typename... more_types>
struct pimpl_detail::unique
{
    // Smart-pointer with the value-semantics behavior.
    // The incomplete-type management technique is originally by Peter Dimov.

    using alloc = typename std::conditional<
                  sizeof...(more_types) == 0,
                  std::allocator<impl_type>,
                  typename first<more_types...>::type>::type;

    template<typename... arg_types>
    void
    make(arg_types&&... args)
    {
        unique u (new impl_type(std::forward<arg_types>(args)...)); // TODO. NEED TO DEPLOY ALLOCATOR
        this->swap(u);
    }

   ~unique () { traits_->destroy(impl_); }
    unique () : traits_(traits()), impl_(0) {}
    unique (impl_type* p) : traits_(deep_copy()), impl_(p) {}
    unique (unique const& that) : traits_(that.traits_), impl_(traits_->copy(that.impl_)) {}

    unique& operator= (unique const& that) { traits_ = that.traits_; traits_->assign(impl_, that.impl_); return *this; }
    bool    operator< (unique const& that) const { return this->impl_ < that.impl_; }

    void           reset (impl_type* p) { unique(p).swap(*this); }
    void            swap (unique& that) { std::swap(impl_, that.impl_), std::swap(traits_, that.traits_); }
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

        operator traits const*() { static traits trait; return &trait; }
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
        operator traits const*() { static deep_copy trait; return &trait; }
    };

    traits const* traits_;
    impl_type*      impl_;
};

template<typename user_type, typename... more_types>
struct pimpl
{
    class       implementation;
    template<class> class base;

    using    unique = base<pimpl_detail::unique<implementation, more_types...>>;
    using    shared = base<pimpl_detail::shared<implementation, more_types...>>;
    using   onstack = base<pimpl_detail::onstack<implementation, more_types...>>;
    using  yes_type = boost::type_traits::yes_type;
    using   no_type = boost::type_traits::no_type;
    using  ptr_type = typename std::remove_reference<user_type>::type*;

    template<class Y> static yes_type test (Y*, typename Y::pimpl_type* =nullptr);
    /***************/ static no_type  test (...);

    BOOST_STATIC_CONSTANT(bool, value = (1 == sizeof(test(ptr_type(nullptr)))));

    static user_type null()
    {
        // null_type needs to be declared in pimpl::base.
        // That way null_type below easily matches pimpl_type and,
        // therefore, base::base(null_type) is called correctly.

        using  null_type = typename user_type::null_type;
        using pimpl_type = typename user_type::pimpl_type;

//        static_assert(pimpl<user_type>::value, "");
        static_assert(sizeof(user_type) == sizeof(pimpl_type), "");

        null_type   arg;
        pimpl_type null (arg);

        return *(user_type*) &null;
    }
};

template<class user_type, typename... more_types>
template<class policy_type>
struct pimpl<user_type, more_types...>::base
{
    struct null_type {};

    using implementation = typename pimpl<user_type, more_types...>::implementation;
    using     pimpl_type = base;

    template<class T> using     rm_ref = typename std::remove_reference<T>::type;
    template<class T> using is_base_of = typename std::is_base_of<base, rm_ref<T>>;
    template<class T> using is_derived = typename std::enable_if<is_base_of<T>::value, null_type*>::type;

    bool         operator! () const { return !impl_.get(); }
    explicit operator bool () const { return  impl_.get(); }

    // Comparison Operators.
    // base::op==() transfers the comparison to 'impl_' (std::shared_ptr or pimpl::unique).
    // Consequently, pointer-semantics (shared_ptr-based) pimpls are comparable as there is shared_ptr::op==().
    // However, value-semantics (unique-based) pimpls are NOT COMPARABLE BY DEFAULT -- the standard
    // value-semantics behavior -- as there is no pimpl::unique::op==().
    // If a value-semantics class T needs to be comparable, then it has to provide
    // T::op==(T const&) EXPLICITLY as part of its public interface.
    // Trying to call this base::op==() for unique-based pimpl will fail to compile (no unique::op==())
    // and will indicate that the user forgot to declare T::operator==(T const&).
    bool operator==(pimpl_type const& that) const { return impl_ == that.impl_; }
    bool operator!=(pimpl_type const& that) const { return impl_ != that.impl_; }
    bool operator< (pimpl_type const& that) const { return impl_  < that.impl_; }

    void swap (pimpl_type& that) { impl_.swap(that.impl_); }
//    void swap (policy_type& that) { impl_.swap(that); }

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

    template<typename, typename...> friend class pimpl;

    base (null_type) {}
    base () { impl_.make(); }

    template<class Arg>
    base(Arg&& arg, is_derived<Arg> =nullptr) : impl_(arg.impl_) {}

    template<typename... Args>
    base(Args&&... args) { impl_.make(std::forward<Args>(args)...); }

    private: policy_type impl_;
};

namespace boost
{
    template<typename user_type, typename... more_types> using pimpl = ::pimpl<user_type, more_types...>;
}

#endif // AUXILIARY_PIMPL_HPP
