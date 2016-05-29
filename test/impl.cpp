#include "./test.hpp"

Foo
create_foo()
{
    return Foo();
}

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
template<> struct pimpl<Shared>::implementation : boost::noncopyable
{
    typedef implementation this_type;

    implementation () : int_(0)             { trace_ =  "Shared::implementation()"; }
    implementation (int k) : int_(k)        { trace_ =  "Shared::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ =  "Shared::implementation(int, int)"; }
    implementation (Foo&)                   { trace_ =  "Shared::implementation(Foo&)"; }
    implementation (Foo const&)             { trace_ =  "Shared::implementation(Foo const&)"; }
    implementation (Foo*)                   { trace_ =  "Shared::implementation(Foo*)"; }
    implementation (Foo const*)             { trace_ =  "Shared::implementation(Foo const*)"; }
    implementation (this_type const& o)
    :
        int_    (o.int_),
        trace_  ("Shared::implementation(Shared::implementation const&)"),
        id_     (o.id_)
    {}

    int              int_;
    mutable string trace_;
    uid const         id_;
};

template<> struct pimpl<Value>::implementation
{
    typedef implementation this_type;

    implementation () : int_(0)             { trace_ =  "Value::implementation()"; }
    implementation (int k) : int_(k)        { trace_ =  "Value::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ =  "Value::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ =  "Value::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ =  "Value::implementation(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("Value::implementation(Value::implementation const&)")
    {}

    int              int_;
    mutable string trace_;
    uid               id_;
};

string Shared::trace () const { return *this ? (*this)->trace_ : "null"; }
string Value:: trace () const { return *this ? (*this)->trace_ : "null"; }
int    Shared::id () const { return (*this)->id_; }
int    Value:: id () const { return (*this)->id_; }

Shared::Shared () : pimpl_type() {}
Value::  Value () : pimpl_type() {}
Shared::Shared (int k) : pimpl_type(k) {}
Value::  Value (int k) : pimpl_type(k) {}
Shared::Shared (int k, int l) : pimpl_type(k, l) {}

Shared::Shared (Foo&       foo) : pimpl_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const& foo) : pimpl_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo*       foo) : pimpl_type(foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const* foo) : pimpl_type(foo) {} // Make sure 'const' handled properly

static Shared single;

Shared::Shared(singleton_type) : pimpl_type(single) {} // 'single' is used as a Singleton.

bool
Value::operator==(Value const& that) const
{
    (*this)->trace_ = "Value::operator==(Value const&)";
    return (*this)->int_ == that->int_;
}

///////////////////////////////////////////////////
// Testing polymorphism
///////////////////////////////////////////////////

template<> struct pimpl<Base>::implementation
{
    implementation (int k) : base_int_(k), trace_("Base::implementation(int)") {}
    virtual ~implementation ()            { /*printf("Base::~implementation()\n"); */ }

    virtual string call_virtual() { return("Base::call_virtual()"); }

    int      base_int_;
    string trace_;
};

struct Derived1Impl : public pimpl<Base>::implementation
{
    typedef pimpl<Base>::implementation pimpl_type;

    Derived1Impl (int k, int l) : pimpl_type(k), derived_int_(l)
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

struct Derived2Impl : public Derived1Impl
{
    typedef Derived1Impl pimpl_type;

    Derived2Impl (int k, int l, int m) : pimpl_type(k, l), more_int_(m)
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

Base::Base(int k) : pimpl_type(k)
{
}

Derived1::Derived1(int k, int l) : Base(pimpl<Base>::null())
{
    reset(new Derived1Impl(k, l));
}

Derived2::Derived2(int k, int l, int m) : Derived1(pimpl<Derived1>::null())
{
    reset(new Derived2Impl(k, l, m));
}

string const&
Base::trace() const
{
    return (*this)->trace_;
}

string
Base::call_virtual()
{
    return (*this)->call_virtual();
}
