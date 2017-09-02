// Copyright (c) 2008-2018 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef IMPL_PTR_DETAIL_INPLACE_HPP
#define IMPL_PTR_DETAIL_INPLACE_HPP

#include <new>
#include "./detail.hpp"

namespace impl_ptr_policy
{
    template<typename, typename> struct inplace;
    template<size_t s, size_t a =std::size_t(-1)> struct storage
    {
        static size_t constexpr size = s;
        static size_t constexpr alignment = a;
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

template<typename impl_type, typename size_type>
struct impl_ptr_policy::inplace // Proof of concept
{
    using    this_type = inplace;
    using storage_type = boost::aligned_storage<size_type::size, size_type::alignment>;
    using  traits_type = detail::traits::copyable<impl_type, inplace_allocator<>>;
    using   traits_ptr = typename traits_type::pointer;

   ~inplace () { if (traits_) traits_->destroy(get()); }
    inplace (std::nullptr_t) {}
    inplace (this_type const& o) : traits_(o.traits_)
    {
        if (traits_)
            traits_->construct(storage_.address(), *o.get());
    }
    inplace (this_type&& o) : traits_(o.traits_)
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
    inplace(detail::in_place_type, arg_types&&... args)
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

        if (traits_)
        {
            traits_->destroy(get());
            traits_ = nullptr;
        }
        ::new (storage_.address()) derived_type(std::forward<arg_types>(args)...);
        traits_ = traits_type();
    }
    impl_type* get () const { return traits_ ? (impl_type*) storage_.address() : nullptr; }

    private:

    storage_type storage_;
    traits_ptr    traits_ = nullptr;
};

#endif // IMPL_PTR_DETAIL_INPLACE_HPP
