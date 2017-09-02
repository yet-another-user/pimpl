// Copyright (c) 2008-2018 Vladimir Batov.
// Copyright (c) 2017      Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_INPLACE_HPP
#define IMPL_PTR_DETAIL_INPLACE_HPP

#include <new>
#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename impl_type, typename> struct inplace;
    template<size_t s, size_t a =std::size_t(-1)> struct always_storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
        static bool   constexpr has_null_state = false;
        template<typename> struct type;
    };
    template<size_t s, size_t a =std::size_t(-1)> struct storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
        static bool   constexpr has_null_state = true;
        template<typename> struct type;
    };
    template<typename T =void> struct inplace_allocator
    {
        using value_type = T;
        void deallocate(T*, size_t) const noexcept {}
        T* allocate(std::size_t) const { throw std::bad_alloc(); }
        bool operator==(const inplace_allocator&) const noexcept { return true; }
        bool operator!=(const inplace_allocator&) const noexcept { return false; }
    };
}

template<size_t s, size_t a>
template<typename impl_type> struct impl_ptr_policy::always_storage<s, a>::type
{
    using  traits_type = detail::traits::copyable<impl_type, inplace_allocator<>>;
    using   traits_ptr = typename traits_type::pointer;
    using storage_type = boost::aligned_storage<s, a>;

    traits_ptr get_traits () const { return const_cast<type&>(*this).set_traits(); }
    traits_ptr set_traits (const traits_ptr ptr = nullptr)
    {
        static traits_ptr traits;
        // Assuming any non-null instance lives at least as long as any instance of this policy
        if (ptr)
            traits = ptr;
        BOOST_ASSERT(traits != nullptr);
        return traits;
    }

    void const* address() const { return storage_.address(); }
    void      * address()       { return storage_.address(); }

    template<typename derived_type, typename... arg_types>
    void emplace(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sizeof(storage_type),
                "Attempting to construct type larger than storage area");
        static_assert((alignof(storage_type) % alignof(derived_type)) == 0,
                "Attempting to construct type in storage area that does not have an integer multiple of the type's alignment requirement.");

        ::new (storage_.address()) derived_type(std::forward<arg_types>(args)...);
        set_traits(traits_type());
    }

    private:
    storage_type storage_;
};

template<size_t s, size_t a>
template<typename impl_type> struct impl_ptr_policy::storage<s, a>::type
    : private always_storage<s, a>::template type<impl_type>
{
    using         base = typename always_storage<s, a>::template type<impl_type>;
    using  traits_type = typename base::traits_type;
    using   traits_ptr = typename base::traits_ptr;

    using base::address;

    traits_ptr get_traits () const                  { return traits_; }
    void       set_traits (const traits_ptr traits) { traits_ = traits; }

    template<typename derived_type, typename... arg_types>
    void emplace(arg_types&&... args)
    {
        base::template emplace<derived_type>(std::forward<arg_types>(args)...);
        set_traits(traits_type());
    }

    private:
    traits_ptr traits_ = nullptr;
};

template<typename impl_type, typename size_type>
struct impl_ptr_policy::inplace // Proof of concept
{
    using    this_type = inplace;
    using storage_type = typename size_type::template type<impl_type>;

   ~inplace ()
    {
        if (const auto traits = storage_.get_traits())
            traits->destroy(get());
    }
    inplace (std::nullptr_t)
    {
        static_assert(size_type::has_null_state, "Constructing null-state is prohibited.");
    }
    inplace (this_type const& o)
    {
        const auto traits = o.storage_.get_traits();
        if (traits)
            traits->construct(storage_.address(), *o.get());
        storage_.set_traits(traits);
    }
    inplace (this_type&& o)
    {
        const auto traits = o.storage_.get_traits();
        if (traits)
            traits->construct(storage_.address(), std::move(*o.get()));
        storage_.set_traits(traits);
    }
    this_type& operator=(this_type const& o)
    {
        const auto traits = storage_.get_traits();
        const auto o_traits = o.storage_.get_traits();

        /**/ if (!traits && !o_traits);
        else if ( traits &&  o_traits) traits->assign(get(), *o.get());
        else if ( traits && !o_traits) traits->destroy(get());
        else if (!traits &&  o_traits) o_traits->construct(storage_.address(), *o.get());

        storage_.set_traits(o_traits);

        return *this;
    }
    this_type& operator=(this_type&& o)
    {
        const auto traits = storage_.get_traits();
        const auto o_traits = o.storage_.get_traits();

        /**/ if (!traits && !o_traits);
        else if ( traits &&  o_traits) traits->assign(get(), std::move(*o.get()));
        else if ( traits && !o_traits) traits->destroy(get());
        else if (!traits &&  o_traits) o_traits->construct(storage_.address(), std::move(*o.get()));

        storage_.set_traits(o_traits);

        return *this;
    }

    template<typename... arg_types>
    inplace(detail::in_place_type, arg_types&&... args)
    {
        storage_.template emplace<impl_type>(std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename... arg_types>
    void emplace(arg_types&&... args)
    {
        static_assert(size_type::has_null_state, "Emplacing to storage that doesn't support null-state is prohibited.");
        if (const auto traits = storage_.get_traits())
        {
            traits->destroy(get());
            storage_.set_traits(nullptr);
        }
        return storage_.template emplace<derived_type>(std::forward<arg_types>(args)...);
    }

    impl_type* get () const { return storage_.get_traits() ? (impl_type*) storage_.address() : nullptr; }

    private:
    storage_type storage_;
};

#endif // IMPL_PTR_DETAIL_INPLACE_HPP
