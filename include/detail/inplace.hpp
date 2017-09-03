// Copyright (c) 2008 Vladimir Batov.
// Copyright (c) 2017 Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_INPLACE_HPP
#define IMPL_PTR_DETAIL_INPLACE_HPP

#include <boost/core/ignore_unused.hpp>
#include <new>
#include "./detail.hpp"

namespace detail
{
    template<typename, typename, bool> struct basic_inplace;
}

namespace impl_ptr_policy
{
    template<size_t s, size_t a =std::size_t(-1)> struct storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
    };
    template<typename impl_type, typename size_type>
    using        inplace = detail::basic_inplace<impl_type, size_type, /* has_null_state = */ true>;
    template<typename impl_type, typename size_type>
    using always_inplace = detail::basic_inplace<impl_type, size_type, /* has_null_state = */ false>;
}

namespace detail
{
    template<typename T =void> struct inplace_allocator
    {
        using value_type = T;
        [[noreturn]] T* allocate(std::size_t) const { throw std::bad_alloc(); }
        constexpr void deallocate(T*, size_t) const noexcept {}
        constexpr bool operator==(const inplace_allocator&) const noexcept { return true; }
        constexpr bool operator!=(const inplace_allocator&) const noexcept { return false; }
    };
    template<typename, typename> struct static_traits;
    template<typename, typename> struct local_traits;
}

template<typename impl_type, typename storage_type>
struct detail::static_traits
    // Inheriting from storage_type to ensure that starts at offset 0 in this struct
    : storage_type
{
    using  traits_type = traits::copyable<impl_type, inplace_allocator<>>;
    using   traits_ptr = typename traits_type::pointer;

    void construct_traits ()       { traits_ = traits_type::singleton(); }
    traits_ptr get_traits () const { return traits_; }
    void set_traits (const traits_ptr ptr)
    {
        boost::ignore_unused(ptr);
        BOOST_ASSERT(ptr == traits_);
    }

    private:
    static traits_ptr traits_;
};

template<typename impl_type, typename storage_type>
typename detail::static_traits<impl_type, storage_type>::traits_ptr
detail::static_traits<impl_type, storage_type>::traits_ = nullptr;

template<typename impl_type, typename storage_type>
struct detail::local_traits
    // Inheriting from storage_type to ensure that starts at offset 0 in this struct
    : storage_type
{
    using  traits_type = traits::copyable<impl_type, inplace_allocator<>>;
    using   traits_ptr = typename traits_type::pointer;

    void construct_traits ()                     { set_traits(traits_type::singleton()); }
    traits_ptr get_traits () const               { return traits_; }
    void       set_traits (const traits_ptr ptr) { traits_ = ptr; }

    private:
    traits_ptr traits_ = nullptr;
};

template<typename impl_type, typename size_type, bool has_null_state>
struct detail::basic_inplace // Proof of concept
{
    using           this_type = basic_inplace;
    using        storage_type = boost::aligned_storage<size_type::size, size_type::alignment>;
    using traits_storage_type = typename std::conditional<has_null_state
                                    , local_traits <impl_type, storage_type>
                                    , static_traits<impl_type, storage_type>
                                    >::type;

   ~basic_inplace ()
    {
        const auto traits = traits_storage_.get_traits();
        if (!has_null_state || traits)
            traits->destroy(get());
    }
    basic_inplace (std::nullptr_t)
    {
        static_assert(has_null_state, "Constructing null-state is prohibited.");
    }
    basic_inplace (this_type const& o)
    {
        const auto traits = o.traits_storage_.get_traits();
        if (!has_null_state || traits)
            traits->construct(traits_storage_.address(), *o.get());
        traits_storage_.set_traits(traits);
    }
    basic_inplace (this_type&& o)
    {
        const auto traits = o.traits_storage_.get_traits();
        if (!has_null_state || traits)
            traits->construct(traits_storage_.address(), std::move(*o.get()));
        traits_storage_.set_traits(traits);
    }
    this_type& operator=(this_type const& o)
    {
        const auto traits = traits_storage_.get_traits();
        const auto o_traits = o.traits_storage_.get_traits();

        /**/ if (!has_null_state
             ||  (traits &&  o_traits)) traits->assign(get(), *o.get());
        else if (!traits && !o_traits);
        else if ( traits && !o_traits) traits->destroy(get());
        else if (!traits &&  o_traits) o_traits->construct(traits_storage_.address(), *o.get());

        traits_storage_.set_traits(o_traits);

        return *this;
    }
    this_type& operator=(this_type&& o)
    {
        const auto traits = traits_storage_.get_traits();
        const auto o_traits = o.traits_storage_.get_traits();

        /**/ if (!has_null_state
             ||  (traits &&  o_traits)) traits->assign(get(), std::move(*o.get()));
        else if (!traits && !o_traits);
        else if ( traits && !o_traits) traits->destroy(get());
        else if (!traits &&  o_traits) o_traits->construct(traits_storage_.address(), std::move(*o.get()));

        traits_storage_.set_traits(o_traits);

        return *this;
    }

    template<typename... arg_types>
    basic_inplace(detail::in_place_type, arg_types&&... args)
    {
        _construct<impl_type>(std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename... arg_types>
    void emplace(arg_types&&... args)
    {
        static_assert(has_null_state, "Emplacing to storage that doesn't support null-state is prohibited.");
        if (const auto traits = traits_storage_.get_traits())
        {
            traits->destroy(get());
            traits_storage_.set_traits(nullptr);
        }
        return _construct<derived_type>(std::forward<arg_types>(args)...);
    }

    impl_type* get () const { return (!has_null_state || traits_storage_.get_traits()) ? (impl_type*) traits_storage_.address() : nullptr; }

    private:
    template<typename derived_type, typename... arg_types>
    void _construct(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sizeof(storage_type),
                "Attempting to construct type larger than storage area");
        static_assert((alignof(storage_type) % alignof(derived_type)) == 0,
                "Attempting to construct type in storage area that does not have an integer multiple of the type's alignment requirement.");

        ::new (traits_storage_.address()) derived_type(std::forward<arg_types>(args)...);
        traits_storage_.construct_traits();
    }

    traits_storage_type traits_storage_;
};

#endif // IMPL_PTR_DETAIL_INPLACE_HPP
