#include "./test.hpp"

namespace { namespace local
{
    static int uid_counter;

    struct uid
    {
        uid() : id_(++uid_counter) {}

        uid(uid const&) =delete;
        uid& operator=(uid const&) =delete;

        operator int() const { return id_; }

        private: int const id_;
    };
}}

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
// Shared
///////////////////////////////////////////////////

// This internal implementation usually only have destructor, constructors,
// data and probably internal methods. Given it is already internal by design,
// it does not need to be a 'class'. All public methods are declared in the
// external visible Shared class. Then, data in this structure are accessed as
// (*this)->data or (**this).data.
template<> struct boost::impl_ptr<Shared>::implementation
{
    using this_type = implementation;

    implementation ()                       { trace_ = "Shared::implementation()"; }
    implementation (int k) : int_(k)        { trace_ = "Shared::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Shared::implementation(int, int)"; }
    implementation (Foo&)                   { trace_ = "Shared::implementation(Foo&)"; }
    implementation (Foo const&)             { trace_ = "Shared::implementation(Foo const&)"; }
    implementation (Foo*)                   { trace_ = "Shared::implementation(Foo*)"; }
    implementation (Foo const*)             { trace_ = "Shared::implementation(Foo const*)"; }

    implementation(this_type const&) =delete;
    this_type& operator=(this_type const&) =delete;

    static Shared create_single()
    {
        Shared single = impl_ptr<Shared>::null();
        single.emplace<implementation>();
        return single;
    }

    void* operator new(size_t sz)
    {
        BOOST_ASSERT(0); // Never called. Use allocators for custom mem. mgmt
        return malloc(sz);
    };
    void operator delete(void* p, size_t)
    {
        BOOST_ASSERT(0); // Never called. Use allocators for custom mem. mgmt
        if (p) free(p);
    }
    int             int_ = 0;
    std::string   trace_;
    local::uid const id_;
};

string Shared::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Shared::value () const { return (*this)->int_; }
int    Shared::   id () const { return (*this)->id_; }

Shared::Shared ()               : impl_ptr_type(in_place) {}
Shared::Shared (int k)          : impl_ptr_type(in_place, k) {}
Shared::Shared (int k, int l)   : impl_ptr_type(in_place, k, l) {}
Shared::Shared (Foo&       foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const& foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo*       foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const* foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (singleton_type) : impl_ptr_type(impl_ptr<Shared>::null())
{
  static Shared single = implementation::create_single();
  *this = single;
}

///////////////////////////////////////////////////
// Copied
///////////////////////////////////////////////////

template<> struct impl_ptr<Copied>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "Copied::implementation()"; }
    implementation (int k) : int_(k)        { trace_ = "Copied::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Copied::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "Copied::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "Copied::implementation(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("Copied::implementation(Copied::implementation const&)")
    {}

    void* operator new(size_t sz)
    {
        return malloc(sz);
    };
    void operator delete(void* p, size_t)
    {
        if (p) free(p);
    }
    int              int_;
    mutable string trace_;
    local::uid        id_;
};

Copied::Copied ()      : impl_ptr_type(in_place) {}
Copied::Copied (int k) : impl_ptr_type(in_place, k) {}

string Copied::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Copied::value () const { return (*this)->int_; }
int    Copied::   id () const { return (*this)->id_; }

bool
Copied::operator==(Copied const& that) const
{
    (*this)->trace_ = "Copied::operator==(Copied const&)";

    BOOST_ASSERT((*this)->id_ != that->id_);

    return (*this)->int_ == that->int_;
}

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
    local::uid        id_;
};

Unique::Unique ()      : impl_ptr_type(in_place) {}
Unique::Unique (int k) : impl_ptr_type(in_place, k) {}

string Unique::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Unique::value () const { return (*this)->int_; }
int    Unique::   id () const { return (*this)->id_; }

///////////////////////////////////////////////////
// Testing polymorphism
///////////////////////////////////////////////////

template<> struct impl_ptr<Base>::implementation
{
    implementation (int k) : base_int_(k), trace_("Base::implementation(int)") {}
    virtual ~implementation() =default;

    virtual string call_virtual() { return("Base::call_virtual()"); }

    void* operator new(size_t sz)
    {
        return malloc(sz);
    };
    void operator delete(void* p, size_t)
    {
        if (p) free(p);
    }
    int base_int_;
    string trace_;
};

struct Derived1Impl : impl_ptr<Base>::implementation
{
    using this_impl = Derived1Impl;
    using base_impl = impl_ptr<Base>::implementation;

    Derived1Impl (int k, int l) : base_impl(k), derived_int_(l)
    {
        BOOST_TEST(trace_ == "Base::implementation(int)");
        trace_ = "Derived1::implementation(int, int)";
    }
   ~Derived1Impl ()
    {
        // printf("Derived1::~implementation()\n");
    }
    virtual string call_virtual() { return ("Derived1::call_virtual()"); }

    int derived_int_;
};

struct Derived2Impl : Derived1Impl
{
    typedef Derived1Impl base_impl;
    typedef Derived2Impl this_impl;

    Derived2Impl (int k, int l, int m) : base_impl(k, l), more_int_(m)
    {
        BOOST_TEST(trace_ == "Derived1::implementation(int, int)");
        trace_ = "Derived2::implementation(int, int, int)";
    }
   ~Derived2Impl ()
    {
//        printf("Derived2::~implementation()\n");
    }
    virtual string call_virtual() { return ("Derived2::call_virtual()"); }

    int more_int_;
};

Base::Base(int k) : impl_ptr_type(in_place, k)
{
}

Derived1::Derived1(int k, int l) : Base(impl_ptr<Base>::null())
{
    emplace<Derived1Impl>(k, l);
}

Derived2::Derived2(int k, int l, int m) : Derived1(impl_ptr<Derived1>::null())
{
    emplace<Derived2Impl>(k, l, m);
}

string
Base::trace() const
{
    return *this ? (*this)->trace_ : "null";
}

string
Base::call_virtual()
{
    return (*this)->call_virtual();
}
