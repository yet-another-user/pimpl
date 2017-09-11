// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_INPLACE_HPP
#define IMPL_PTR_DETAIL_INPLACE_HPP

#include <boost/compressed_pair.hpp>
#include <boost/config.hpp>
#include <boost/core/ignore_unused.hpp>
#include <new>
#include "./detail.hpp"

namespace detail
{
    template<typename, typename, typename> struct basic_inplace;
    struct exists_always;
}

namespace impl_ptr_policy
{
    template<size_t s, size_t a =std::size_t(-1)> struct storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
    };
    template<typename impl_type, typename size_type>
    using        inplace = detail::basic_inplace<impl_type, size_type, /* exists_type = */ bool>;
    template<typename impl_type, typename size_type>
    using always_inplace = detail::basic_inplace<impl_type, size_type, /* exists_type = */ detail::exists_always>;
}

namespace detail
{
    template<typename T =void> struct inplace_allocator
    {
        using value_type = T;
        [[noreturn]] T* allocate(std::size_t) const { throw std::bad_alloc(); }
        BOOST_CXX14_CONSTEXPR void deallocate(T*, size_t) const noexcept {}
        constexpr bool operator==(const inplace_allocator&) const noexcept { return true; }
        constexpr bool operator!=(const inplace_allocator&) const noexcept { return false; }
    };
}

struct detail::exists_always
{
    constexpr explicit exists_always(bool) {}
    constexpr operator bool() const { return true; }
    constexpr const exists_always& operator=(bool exists) const
    {
        return exists
            ? *this
            : (throw std::invalid_argument("exists_always: setting to non-existent prohibited"), *this)
            ;
    }
};

static_assert(std::is_empty<detail::exists_always>::value, "detail::exists_always causes memory overhead");

template<typename impl_type, typename size_type, typename exists_type>
struct detail::basic_inplace // Proof of concept
{
    using    this_type = basic_inplace;
    using storage_type = boost::aligned_storage<size_type::size, size_type::alignment>;
    using  traits_type = traits::copyable<impl_type, inplace_allocator<>>;
    using   alloc_type = typename traits_type::alloc_type;

   ~basic_inplace ()
    {
        if (exists())
            traits_type::destroy(get());
    }
    BOOST_CXX14_CONSTEXPR basic_inplace (std::nullptr_t)
    {
        static_assert(exists_type(false) == false, "Constructing null-state is prohibited.");
    }
    BOOST_CXX14_CONSTEXPR basic_inplace (this_type const& o)
    {
        _assign(o);
    }
    BOOST_CXX14_CONSTEXPR basic_inplace (this_type&& o)
    {
        _assign(std::move(o));
    }
    BOOST_CXX14_CONSTEXPR this_type& operator=(this_type const& o)
    {
        return _assign(o);
    }
    BOOST_CXX14_CONSTEXPR this_type& operator=(this_type&& o)
    {
        return _assign(std::move(o));
    }

    template<typename... arg_types>
    basic_inplace(detail::in_place_type, arg_types&&... args)
    {
        _construct<impl_type>(std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename... arg_types>
    void emplace(arg_types&&... args)
    {
        static_assert(exists_type(false) == false, "Emplacing to storage that doesn't support null-state is prohibited.");
        if (exists())
        {
            set_exists(false);
            traits_type::destroy(get());
        }
        return _construct<derived_type>(std::forward<arg_types>(args)...);
    }

    impl_type* get () const { return exists() ? (impl_type*) storage().address() : nullptr; }

    private:
    template<typename derived_type, typename... arg_types>
    void _construct(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sizeof(storage_type),
                "Attempting to construct type larger than storage area");
        static_assert((alignof(storage_type) % alignof(derived_type)) == 0,
                "Attempting to construct type in storage area that does not have an integer multiple of the type's alignment requirement.");

        using alloc_type = typename std::allocator_traits<basic_inplace::alloc_type>::template rebind_alloc<derived_type>;
        alloc_type a;
        traits_type::emplace(a, static_cast<derived_type*>(storage().address()), std::forward<arg_types>(args)...);
        set_exists(true);
    }

    template<typename T>
    BOOST_CXX14_CONSTEXPR
    typename std::enable_if<std::is_same<typename std::decay<T>::type, this_type>::value, this_type&>::type
    _assign(T&& o)
    {
        using uref = typename std::conditional<std::is_lvalue_reference<T>::value, const impl_type&, impl_type>::type;

        const bool   exists = this->exists();
        const bool o_exists =     o.exists();

        /**/ if (!exists && !o_exists);
        else if ( exists &&  o_exists) traits_type::assign(get(), std::forward<uref>(*o.get()));
        else if ( exists && !o_exists) { set_exists(false); traits_type::destroy(get()); }
        else if (!exists &&  o_exists) { traits_type::construct(storage().address(), std::forward<uref>(*o.get())); set_exists(true); }

        return *this;
    }

    BOOST_CXX14_CONSTEXPR storage_type& storage()
    {
        return storage_and_state_.first();
    }

    constexpr const storage_type& storage() const
    {
        return storage_and_state_.first();
    }

    constexpr bool exists() const
    {
        return storage_and_state_.second();
    }

    BOOST_CXX14_CONSTEXPR void set_exists(bool exists)
    {
        storage_and_state_.second() = exists;
    }

    boost::compressed_pair<
        // storage_type must be the first here to ensure it starts at offset 0 in this struct
        storage_type
      , exists_type
      > storage_and_state_ { exists_type(false) };
};

#endif // IMPL_PTR_DETAIL_INPLACE_HPP
