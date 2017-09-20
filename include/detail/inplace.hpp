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
    template<typename, typename, typename, typename> struct basic_inplace;
    struct exists_always;
}

namespace impl_ptr_policy
{
    template<size_t s, size_t a =std::size_t(-1)> struct storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
    };
    template<typename impl_type, typename size_type, typename allocator =std::allocator<void>>
    using        inplace = detail::basic_inplace<impl_type, size_type, allocator, /* exists_type = */ bool>;
    template<typename impl_type, typename size_type, typename allocator =std::allocator<void>>
    using always_inplace = detail::basic_inplace<impl_type, size_type, allocator, /* exists_type = */ detail::exists_always>;
}

namespace detail
{
    template<typename T, typename inner_alloc> struct inplace_allocator
    {
        using           value_type = T;

        [[noreturn]] T* allocate(std::size_t) const { throw std::bad_alloc(); }
        BOOST_CXX14_CONSTEXPR void deallocate(T*, size_t) const noexcept {}
        constexpr bool operator==(const inplace_allocator& o) const noexcept
        {
            return !inner_allocator
                ? !o.inner_allocator
                : !o.inner_allocator
                ? !inner_allocator
                : *inner_allocator == *o.inner_allocator
                ;
        }
        constexpr bool operator!=(const inplace_allocator& o) const noexcept { return !(*this == o); }

        constexpr inplace_allocator() noexcept : inner_allocator(nullptr) {}
        constexpr inplace_allocator(const inner_alloc& a) noexcept : inner_allocator(&a) {}

        template<typename derived_type, typename... arg_types>
        void construct(derived_type* p, arg_types&&... args)
        {
            if (inner_allocator)
                construct_with_inner_allocator(use_alloc<derived_type, inner_alloc, arg_types...>{},
                  p, std::forward<arg_types>(args)...);
            else
                construct_with_inner_allocator(construct_without_alloc{},
                  p, std::forward<arg_types>(args)...);
        }

        private:
        struct construct_without_alloc {};
        struct construct_with_alloc_tag {};
        struct construct_with_alloc_last {};

        template <typename TT, typename Alloc, typename... Args>
        using use_alloc = typename std::conditional<
            std::uses_allocator<TT, Alloc>::value && std::is_constructible<TT, std::allocator_arg_t, Alloc, Args...>::value
          , construct_with_alloc_tag
          , typename std::conditional<
                std::uses_allocator<TT, Alloc>::value && std::is_constructible<TT, Args..., Alloc>::value
              , construct_with_alloc_last
              , construct_without_alloc
              >::type
          >::type;

        template<typename derived_type, typename... arg_types>
        void construct_with_inner_allocator(construct_without_alloc, derived_type* p, arg_types&&... args)
        {
            ::new (static_cast<void*>(p)) derived_type(std::forward<arg_types>(args)...);
        }

        template<typename derived_type, typename... arg_types>
        void construct_with_inner_allocator(construct_with_alloc_last, derived_type* p, arg_types&&... args)
        {
            ::new (static_cast<void*>(p)) derived_type(
                std::forward<arg_types>(args)...
              , *inner_allocator);
        }

        template<typename derived_type, typename... arg_types>
        void construct_with_inner_allocator(construct_with_alloc_tag, derived_type* p, arg_types&&... args)
        {
            ::new (static_cast<void*>(p)) derived_type(
                std::allocator_arg, *inner_allocator
              , std::forward<arg_types>(args)...);
        }

        const inner_alloc* inner_allocator;
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

template<typename impl_type, typename size_type, typename allocator, typename exists_type>
struct detail::basic_inplace // Proof of concept
{
    using      this_type = basic_inplace;
    using   storage_type = boost::aligned_storage<size_type::size, size_type::alignment>;
    using    traits_type = traits::copyable<impl_type, inplace_allocator<impl_type, allocator>>;
    using allocator_type = typename traits_type::alloc_type;

   ~basic_inplace ()
    {
        allocator_type a;
        if (exists())
            traits_type::destroy(a, get());
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

    template<typename alloc_arg, typename... arg_types>
    basic_inplace(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        _construct<impl_type>(std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename alloc_arg, typename... arg_types>
    void emplace(std::allocator_arg_t, alloc_arg&& a, arg_types&&... args)
    {
        static_assert(exists_type(false) == false, "Emplacing to storage that doesn't support null-state is prohibited.");
        if (exists())
        {
            allocator_type d;
            set_exists(false);
            traits_type::destroy(d, get());
        }
        return _construct<derived_type>(std::forward<alloc_arg>(a), std::forward<arg_types>(args)...);
    }

    impl_type* get () const { return exists() ? (impl_type*) storage().address() : nullptr; }

    private:
    template<typename derived_type, typename alloc_arg, typename... arg_types>
    void _construct(alloc_arg&& a0, arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sizeof(storage_type),
                "Attempting to construct type larger than storage area");
        static_assert((alignof(storage_type) % alignof(derived_type)) == 0,
                "Attempting to construct type in storage area that does not have an integer multiple of the type's alignment requirement.");

        using alloc_type = typename std::allocator_traits<basic_inplace::allocator_type>::template rebind_alloc<derived_type>;
        alloc_type a(std::forward<alloc_arg>(a0));
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

        allocator_type a;
        /**/ if (!exists && !o_exists);
        else if ( exists &&  o_exists) traits_type::assign(get(), std::forward<uref>(*o.get()));
        else if ( exists && !o_exists) { set_exists(false); traits_type::destroy(a, get()); }
        else if (!exists &&  o_exists) { traits_type::construct(a, storage().address(), std::forward<uref>(*o.get())); set_exists(true); }

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
