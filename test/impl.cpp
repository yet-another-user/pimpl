#include "./test.hpp"

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
    void construct(pointer p, const T& value)
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

///////////////////////////////////////////////////
// Book
///////////////////////////////////////////////////

template<> struct boost::impl_ptr<Book>::implementation
{
    implementation(string const& the_title, string const& the_author)
    :
        title(the_title), author(the_author)
    {}

    string  title;
    string author;
};

Book::Book(string const& title, string const& author)
:
//  impl_ptr_type(in_place, title, author)
    impl_ptr_type(in_place, std::allocator<implementation>(), title, author)
{
    // Various ways of initializing the impl_ptr base:
    // 1) Internally calls std::make_shared
    // 2) Internally calls std::allocate_shared(allocator, ...)
    // 3) Calling std::make_shared explicitly.
    // 4) Calling std::allocate_shared explicitly.
}

string const& Book:: title() const { return (*this)->title; }
string const& Book::author() const { return (*this)->author; }

///////////////////////////////////////////////////
// Unique
///////////////////////////////////////////////////

template<> struct impl_ptr<Unique>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "Unique::implementation()"; }
    implementation (int k) : int_(k)        { trace_ = "Unique::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Unique::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "Unique::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "Unique::implementation(Foo const&)"; }

    int              int_;
    mutable string trace_;
};

Unique::Unique ()      : impl_ptr_type(in_place) {}
Unique::Unique (int k) : impl_ptr_type(in_place, k) {}

string Unique::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Unique::value () const { return (*this)->int_; }

