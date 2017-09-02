// Copyright (c) 2008-2018 Vladimir Batov.
// Copyright (c) 2017      Giel van Schijndel.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_ONSTACK_HPP
#define IMPL_PTR_DETAIL_ONSTACK_HPP

#include <type_traits>
#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename, typename =void> struct onstack;
}

template<typename impl_type, typename size_type, typename align_type>
struct impl_ptr_policy::onstack // Proof of concept
{
    template<typename T =void> struct allocator : std::allocator<T>
    {
        void deallocate(T*, size_t) {}

        template<typename Y> struct rebind { using other = allocator<Y>; };
    };

    template <typename _size_type, typename _align_type = void>
    struct aligned_storage
    {
        using type = boost::aligned_storage<sizeof(_size_type), alignof(_align_type)>;
    };

    template <typename _size_type>
    struct aligned_storage<_size_type, void>
    {
        // void: explicitly ask for default alignment for this size
        using type = boost::aligned_storage<sizeof(_size_type)>;
    };

    template <typename SizeInteger, SizeInteger size, typename AlignInteger, AlignInteger align>
    struct aligned_storage<std::integral_constant<SizeInteger, size>, std::integral_constant<AlignInteger, align>>
    {
        using type = boost::aligned_storage<size, align>;
    };

    template <typename SizeInteger, SizeInteger size, typename _align_type>
    struct aligned_storage<std::integral_constant<SizeInteger, size>, std::alignment_of<_align_type>>
    {
        using type = boost::aligned_storage<size, alignof(_align_type)>;
    };

    template <typename SizeInteger, SizeInteger size>
    struct aligned_storage<std::integral_constant<SizeInteger, size>, void>
    {
        // void: explicitly ask for default alignment for this size
        using type = boost::aligned_storage<size>;
    };

    using    this_type = onstack;
    using storage_type = typename aligned_storage<size_type, align_type>::type;
    using  traits_type = detail::traits::copyable<impl_type, allocator<>>;
    using   traits_ptr = typename traits_type::pointer;

   ~onstack () { if (traits_) traits_->destroy(get()); }
    onstack (std::nullptr_t) {}
    onstack (this_type const& o) : traits_(o.traits_)
    {
        if (traits_)
            traits_->construct(storage_.address(), *o.get());
    }
    onstack (this_type&& o) : traits_(o.traits_)
    {
        if (traits_)
            traits_->construct(storage_.address(), std::move(*o.get()));
    }
    this_type& operator=(this_type const& o)
    {
        /**/ if (!traits_ && !o.traits_);
        else if ( traits_ &&  o.traits_) traits_->assign(get(), *o.get());
        else if ( traits_ && !o.traits_) traits_->destroy(get());
        else if (!traits_ &&  o.traits_) o.traits_->construct(storage_.address(), *o.get());

        traits_ = o.traits_;

        return *this;
    }
    this_type& operator=(this_type&& o)
    {
        /**/ if (!traits_ && !o.traits_);
        else if ( traits_ &&  o.traits_) traits_->assign(get(), std::move(*o.get()));
        else if ( traits_ && !o.traits_) traits_->destroy(get());
        else if (!traits_ &&  o.traits_) o.traits_->construct(storage_.address(), std::move(*o.get()));

        traits_ = o.traits_;

        return *this;
    }

    template<typename... arg_types>
    onstack(detail::in_place_type, arg_types&&... args)
    {
        emplace<impl_type>(std::forward<arg_types>(args)...);
    }

    template<typename derived_type, typename... arg_types>
    void
    emplace(arg_types&&... args)
    {
        static_assert(sizeof(derived_type) <= sizeof(storage_type),
                "Attempting to construct type larger than storage area");
        static_assert((alignof(storage_type) % alignof(derived_type)) == 0,
                "Attempting to construct type in storage area that does not have an integer multiple of the type's alignment requirement.");

        ::new (storage_.address()) derived_type(std::forward<arg_types>(args)...);
        traits_ = traits_type();
    }
    impl_type* get () const { return traits_ ? (impl_type*) storage_.address() : nullptr; }

    private:

    storage_type storage_;
    traits_ptr    traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_ONSTACK_HPP
