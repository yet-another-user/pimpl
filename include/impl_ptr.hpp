// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_HPP
#define IMPL_PTR_HPP

#include "./detail/shared.hpp"
#include "./detail/unique.hpp"
#include "./detail/copied.hpp"
#include "./detail/onstack.hpp"

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

template<
    typename user_type,
    template<typename, typename...> class PT =detail::no_policy,
    typename... more_types>
struct impl_ptr
{
    template<typename... MT>
    using onstack = impl_ptr<user_type, impl_ptr_policy::onstack, MT...>;
    using  shared = impl_ptr<user_type, impl_ptr_policy:: shared>;
    using  unique = impl_ptr<user_type, impl_ptr_policy:: unique>;
    using  copied = impl_ptr<user_type, impl_ptr_policy:: copied>;

    struct implementation;

    using impl_ptr_type = impl_ptr;
    using     impl_type = typename impl_ptr<user_type>::implementation; //C1
    using   policy_type = PT<impl_type, more_types...>;

    static user_type null()
    {
        using impl_ptr_type = typename user_type::impl_ptr_type;

        static_assert(sizeof(user_type) == sizeof(impl_ptr_type), "Unsafe to cast");

        return std::move(static_cast<user_type&&>(impl_ptr_type(nullptr)));
    }

    static constexpr detail::in_place_type in_place {}; // Until C++17 with std::in_place

   ~impl_ptr()                           = default;
    impl_ptr(impl_ptr const&)            = default;
    impl_ptr(impl_ptr&&)                 = default;
    impl_ptr& operator=(impl_ptr const&) = default;
    impl_ptr& operator=(impl_ptr&&)      = default;

    bool         operator! () const { return !impl_.get(); }
    explicit operator bool () const { return  impl_.get(); }

    bool operator==(user_type const& that) const { return impl_ == that.impl_; } //C2
    bool operator!=(user_type const& that) const { return impl_ != that.impl_; } //C2
    bool operator< (user_type const& that) const { return impl_  < that.impl_; }

    void      swap (user_type& that) { impl_.swap(that.impl_); }
    long use_count () const { return impl_.use_count(); }

    template<typename derived_impl_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        static_assert(std::is_base_of<impl_type, derived_impl_type>::value, "");

        impl_.template emplace<derived_impl_type>(std::forward<arg_types>(args)...);
    }
    template<typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        impl_.template emplace<impl_type>(std::forward<arg_types>(args)...);
    }

    // Access To the Implementation.
    // 1) These methods are public because they are only usable
    //    in the code where impl_ptr<>::implementation is visible.
    // 2) For better or worse the original deep-constness behavior has been changed
    //    to match std::shared_ptr et al to avoid questions, confusion, etc.
    impl_type* operator->() const { BOOST_ASSERT(impl_.get()); return  impl_.get(); }
    impl_type& operator *() const { BOOST_ASSERT(impl_.get()); return *impl_.get(); }

    protected:

    template<typename, template<typename, typename...> class, typename...> friend struct impl_ptr;

    impl_ptr(std::nullptr_t) : impl_(nullptr) {}

    template<typename... arg_types>
    impl_ptr(detail::in_place_type, arg_types&&... args)
    :
        impl_(in_place, std::forward<arg_types>(args)...)
    {}

    private: policy_type impl_;
};

namespace boost
{
    template <typename ...> using void_type = void;

    template<typename U, template<typename, typename...> class P =::detail::no_policy, typename... M>
    using impl_ptr = ::impl_ptr<U, P, M...>;

    template<typename, typename =void>
    struct is_impl_ptr : false_type {};

    template<typename T>
    struct is_impl_ptr<T, void_type<typename T::impl_ptr_type>> : true_type {};
}

#endif // IMPL_PTR_HPP
