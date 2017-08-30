// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_HPP
#define IMPL_PTR_HPP

#include "./detail/shared.hpp"
#include "./detail/unique.hpp"
#include "./detail/copied.hpp"
#include "./detail/onstack.hpp"
#include "./detail/cow.hpp"

// C1. Always use the impl_ptr<user_type>::implementation specialization.
//     That allows the developer to only declare/define one implementation:
//         template<> struct impl_ptr<user_type>::implementation { ... };
//     regardless of the extra types/args passed in externally.
//     That (obviously) simplifies the internal implementation.
// C2. Comparison Operators.
//     base::op==() transfers the comparison to 'impl_'. Consequently,
//     shared_ptr-based pimpls are comparable due to shared_ptr::op==().
//     However, value-semantics (unique-based) pimpls are NOT COMPARABLE BY DEFAULT --
//     the standard value-semantics behavior -- due to NO unique::op==().
//     If a value-semantics class T needs to be comparable, then it has to provide
//     T::op==(T const&) EXPLICITLY as part of its own public interface.
//     Trying to call this base::op==() for unique-based impl_ptr will fail to compile
//     (no unique::op==()) and will indicate that the user forgot to declare
//     T::operator==(T const&).

template<typename user_type, typename... more_types>
struct boost_impl_ptr_detail
{
    struct          implementation;
    template<typename> struct base;

    using impl_type = typename boost_impl_ptr_detail<user_type>::implementation; //C1

    template<template<typename IT, typename... MT> typename PT>
    using policy = base<PT<impl_type, more_types...>>;

    using  shared = policy<detail:: shared>;
    using  unique = policy<detail:: unique>;
    using  copied = policy<detail:: copied>;
    using onstack = policy<detail::onstack>;
    using     cow = policy<detail::    cow>;

    static user_type null()
    {
        using impl_ptr_type = typename user_type::impl_ptr_type;

        static_assert(sizeof(user_type) == sizeof(impl_ptr_type), "Unsafe to cast");

        return std::move(static_cast<user_type&&>(impl_ptr_type(nullptr)));
    }
};

template<typename user_type, typename... more_types>
template<typename policy_type>
struct boost_impl_ptr_detail<user_type, more_types...>::base
{
    using implementation = typename boost_impl_ptr_detail<user_type>::implementation; //C1
    using  impl_ptr_type = base;

    static constexpr detail::in_place_type in_place {}; // Until C++17 with std::in_place

   ~base()                       = default;
    base(base const&)            = default;
    base(base&&)                 = default;
    base& operator=(base const&) = default;
    base& operator=(base&&)      = default;

    bool         operator! () const { return !impl_.get(); }
    explicit operator bool () const { return  impl_.get(); }

    bool operator==(user_type const& that) const { return impl_ == that.impl_; } //C2
    bool operator!=(user_type const& that) const { return impl_ != that.impl_; } //C2
    bool operator< (user_type const& that) const { return impl_  < that.impl_; }

    void      swap (user_type& that) { impl_.swap(that.impl_); }
    long use_count () const { return impl_.use_count(); }

    template<typename impl_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        static_assert(std::is_base_of<implementation, impl_type>::value, "");

        impl_.template emplace<impl_type>(std::forward<arg_types>(args)...);
    }
    template<typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        impl_.template emplace<implementation>(std::forward<arg_types>(args)...);
    }

    // Access To the Implementation.
    // 1) These methods are public because they are only usable
    //    in the code where impl_ptr<>::implementation is visible.
    // 2) For better or worse the original deep-constness behavior has been changed
    //    to match std::shared_ptr et al to avoid questions, confusion, etc.
    implementation* operator->() const { BOOST_ASSERT(impl_.get()); return  impl_.get(); }
    implementation& operator *() const { BOOST_ASSERT(impl_.get()); return *impl_.get(); }

    protected:

    template<typename, typename...> friend struct boost_impl_ptr_detail;

    base(std::nullptr_t) : impl_(nullptr) {}

    template<typename... arg_types>
    base(detail::in_place_type, arg_types&&... args)
    :
        impl_(in_place, std::forward<arg_types>(args)...)
    {}

    private: policy_type impl_;
};

namespace boost
{
    template <typename ...> using void_type = void;
    template<typename U, typename... M> using impl_ptr = ::boost_impl_ptr_detail<U, M...>;

    template<typename, typename =void>
    struct is_impl_ptr : false_type {};

    template<typename T>
    struct is_impl_ptr<T, void_type<typename T::impl_ptr_type>> : true_type {};
}

#endif // IMPL_PTR_HPP
