// Copyright (c) 2006-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_HPP
#define IMPL_PTR_HPP

#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

#include "./detail/shared.hpp"
#include "./detail/unique.hpp"
#include "./detail/copied.hpp"
#include "./detail/unique_au.hpp"
#include "./detail/copied_au.hpp"
#include "./detail/onstack.hpp"
#include "./detail/cow.hpp"

namespace detail
{
    struct     null_type {};
    struct in_place_type {};

    constexpr null_type null_arg {};
}

// C1. Always use the impl_ptr<user_type>::implementation specialization.
//     That allows the implementation developer to only declare/define one
//     implementation:
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
struct impl_ptr
{
    struct          implementation;
    template<typename> struct base;

    using impl_type = typename impl_ptr<user_type>::implementation; //C1
    using    shared = base<detail::shared    <impl_type, more_types...>>;
    using    unique = base<detail::unique    <impl_type, more_types...>>;
    using unique_au = base<detail::unique_au <impl_type, more_types...>>;
    using    copied = base<detail::copied    <impl_type, more_types...>>;
    using copied_au = base<detail::copied_au <impl_type, more_types...>>;
    using   onstack = base<detail::onstack   <impl_type, more_types...>>;
    using       cow = base<detail::cow       <impl_type, more_types...>>;

    static user_type null()
    {
        using impl_ptr_type = typename user_type::impl_ptr_type;

        static_assert(sizeof(user_type) == sizeof(impl_ptr_type), "");

        impl_ptr_type null (detail::null_arg);

        return std::move(static_cast<user_type&>(null));
    }
};

template<typename user_type, typename... more_types>
template<typename policy_type>
struct impl_ptr<user_type, more_types...>::base
{
    using implementation = typename impl_ptr<user_type>::implementation; //C1
    using  impl_ptr_type = base;

    static constexpr detail::in_place_type in_place {}; // Until C++17 with std::in_place

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

    template<typename other_type>
    static other_type null() { return impl_ptr<other_type>::null(); }
    static user_type  null() { return impl_ptr< user_type>::null(); }

    protected:

    template<typename, typename...> friend struct impl_ptr;

    base (detail::null_type) {}

    template<typename... arg_types>
    base(detail::in_place_type, arg_types&&... args)
    {
        impl_.template emplace<implementation>(std::forward<arg_types>(args)...);
    }

    private: policy_type impl_;
};

namespace boost
{
    template<typename U, typename... M> using impl_ptr = ::impl_ptr<U, M...>;

    template<typename, typename =void>
    struct is_impl_ptr : false_type {};

    template<typename T>
    struct is_impl_ptr<T, void_t<typename T::impl_ptr_type>> : true_type {};
}

#endif // IMPL_PTR_HPP
