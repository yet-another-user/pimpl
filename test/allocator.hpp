#ifndef IMPL_PTR_TEST_ALLOCATOR_HPP
#define IMPL_PTR_TEST_ALLOCATOR_HPP

#include <memory>

template <class T>
struct my_allocator
{
    using       this_type = my_allocator;
    using       size_type = size_t;
    using difference_type = ptrdiff_t;
    using         pointer = T*;
    using   const_pointer = T const*;
    using       reference = T&;
    using const_reference = T const&;
    using      value_type = T;

   ~my_allocator() throw() {}
    my_allocator() throw() {}

    my_allocator(const my_allocator&) throw() {}

    template<typename other_type>
    my_allocator(my_allocator<other_type> const&) throw() {}

    // allocate but don't initialize num elements of type T
    pointer allocate(size_type num)
    {
        return (pointer) ::operator new(num * sizeof(T));
    }
    // initialize elements of allocated storage p with value value
    void construct(pointer p, T const& value)
    {
        new ((void*) p) T(value);
    }
    // delete elements of initialized storage p
    void destroy(pointer p)
    {
        p->~T();
    }
    // deallocate storage p of deleted elements
    void deallocate(pointer p, size_type num)
    {
        ::operator delete((void*) p);
    }
};

template <class T1, class T2>
bool operator==(my_allocator<T1> const&, my_allocator<T2> const&) throw()
{
    return true;
}

template <class T1, class T2>
bool operator!=(my_allocator<T1> const&, my_allocator<T2> const&) throw()
{
    return false;
}

#endif // IMPL_PTR_TEST_ALLOCATOR_HPP
