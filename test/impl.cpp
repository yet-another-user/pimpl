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

struct deleter
{
    // Convenience deleter allows to "manage" objects without actually managing them.
    struct no { void operator()(void*) {}};
};

template<>
struct boost::impl_ptr<Book>::implementation
{
    implementation(string const& the_title, string const& the_author)
    :
        title(the_title), author(the_author)
    {
    }

    string  title;
    string author;
};

Book::Book() : base_type(null_type())
{
}

Book::Book(string const& title, string const& author)
:
//  base_type(title, author)
    base_type(std::allocator<implementation>(), title, author)
//  base_type(std::make_shared<implementation>(title, author))
//  base_type(std::allocate_shared<implementation>(std::allocator<implementation>(), title, author))
{
    // Various ways of initializing the impl_ptr base:
    // 1) Internally calls std::make_shared
    // 2) Internally calls std::allocate_shared(allocator, ...)
    // 3) Calling std::make_shared explicitly.
    // 4) Calling std::allocate_shared explicitly.
}

string const& Book:: title() const { return (*this)->title; }
string const& Book::author() const { return (*this)->author; }

struct uid
{
    uid() : id_(counter()) {}
    uid(uid const&) : id_(counter()) {}
    uid& operator=(uid const&) { return *this; }

    operator int() const { return id_; }

    private:

    int counter()
    {
        static int counter_;
        return ++counter_;
    }
    int const id_;
};

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

    void* operator new(size_t sz)
    {
        BOOST_ASSERT(0); // Never called. Need allocators for custom mem. mgmt
        return malloc(sz);
    };
    void operator delete(void* p, size_t)
    {
        BOOST_ASSERT(0); // Never called. Need allocators for custom mem. mgmt
        if (p) free(p);
    }
    int           int_ = 0;
    std::string trace_;
    uid const      id_;
};

string Shared::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Shared::value () const { return (*this)->int_; }
int    Shared::   id () const { return (*this)->id_; }

Shared::Shared () {}
Shared::Shared (int k) : base_type(k) {}
Shared::Shared (int k, int l) : base_type(k, l) {}
Shared::Shared (Foo&       foo) : base_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const& foo) : base_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo*       foo) : base_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const* foo) : base_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (singleton_type) : base_type(impl_ptr<base_type>::null())
{
//  If there is an available/suitable public Shared::Shared(...).
//  static Shared single;
//  *this = single;

    static implementation impl;
    reset(&impl, deleter::no());
}

///////////////////////////////////////////////////
// Value
///////////////////////////////////////////////////

template<> struct impl_ptr<Value>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "Value::implementation()"; }
    implementation (int k) : int_(k)        { trace_ = "Value::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Value::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "Value::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "Value::implementation(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("Value::implementation(Value::implementation const&)")
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
    uid               id_;
};

Value::Value () {}
Value::Value (int k) : base_type(k) {}

string Value::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Value::id () const { return (*this)->id_; }

bool
Value::operator==(Value const& that) const
{
    (*this)->trace_ = "Value::operator==(Value const&)";

    BOOST_ASSERT((*this)->id_ != that->id_);

    return (*this)->int_ == that->int_;
}

///////////////////////////////////////////////////
// Testing polymorphism
///////////////////////////////////////////////////

template<> struct impl_ptr<Base>::implementation
{
    implementation (int k) : base_int_(k), trace_("Base::implementation(int)") {}
    virtual ~implementation() =default;

    virtual string call_virtual() { return("Base::call_virtual()"); }

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

Base::Base(int k) : base_type(k)
{
}

Derived1::Derived1(int k, int l) : Base(impl_ptr<Base>::null())
{
    reset(new Derived1Impl(k, l));
}

Derived2::Derived2(int k, int l, int m) : Derived1(impl_ptr<Derived1>::null())
{
    reset(new Derived2Impl(k, l, m));
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
