#include "./pimpl_test.hpp"
#include <stdio.h>

typedef std::string string;

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
// external visible Test1 class. Then, data in this structure are accessed as
// (*this)->data or (**this).data.
template<> struct pimpl<Test1>::implementation : boost::noncopyable
{
    typedef implementation this_type;

    implementation () : int_(0)             { trace_ =  "Test1::implementation()"; }
    implementation (int k) : int_(k)        { trace_ =  "Test1::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ =  "Test1::implementation(int, int)"; }
    implementation (Foo&)                   { trace_ =  "Test1::implementation(Foo&)"; }
    implementation (Foo const&)             { trace_ =  "Test1::implementation(Foo const&)"; }
    implementation (Foo      &, Foo      &) { trace_ =  "Test1::implementation(Foo&, Foo&)"; }
    implementation (Foo      &, Foo const&) { trace_ =  "Test1::implementation(Foo&, Foo const&)"; }
    implementation (Foo const&, Foo      &) { trace_ =  "Test1::implementation(Foo const&, Foo&)"; }
    implementation (Foo const&, Foo const&) { trace_ =  "Test1::implementation(Foo const&, Foo const&)"; }
    implementation (Foo*)                   { trace_ =  "Test1::implementation(Foo*)"; }
    implementation (Foo const*)             { trace_ =  "Test1::implementation(Foo const*)"; }
    implementation (this_type const& o)
    :
        int_    (o.int_),
        trace_  ("Test1::implementation(Test1::implementation const&)"),
        id_     (o.id_)
    {}

    int              int_;
    mutable string trace_;
    uid const         id_;
};

template<> struct pimpl<Test2>::implementation
{
    typedef implementation this_type;

    implementation () : int_(0)             { trace_ =  "Test2::implementation()"; }
    implementation (int k) : int_(k)        { trace_ =  "Test2::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ =  "Test2::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ =  "Test2::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ =  "Test2::implementation(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("Test2::implementation(Test2::implementation const&)")
    {}

    int              int_;
    mutable string trace_;
    uid               id_;
};

typedef Foo const& cref;

//Test1::Test1(pass_value_type) : pimpl_type(Foo::create()) {}
Test1::Test1(pass_value_type const&) : pimpl_type(cref(create_foo())) {}

string const& Test1::trace () const { return (*this)->trace_; }
string const& Test2::trace () const { return (*this)->trace_; }
int           Test1::   id () const { return (*this)->id_; }
int           Test2::   id () const { return (*this)->id_; }

Test1::Test1 () : pimpl_type() {}                     // Call implementation::implementation()
Test2::Test2 () : pimpl_type() {}                     // ditto
Test1::Test1 (int k) : pimpl_type(k) {}               // Call implementation::implementation(int)
Test2::Test2 (int k) : pimpl_type(k) {}               // ditto
Test1::Test1 (int k, int l) : pimpl_type(k, l) {}     // Call implementation::implementation(int, int)

Test1::Test1 (Foo&       foo) : pimpl_type(foo) {}                  // Make sure 'const' handled properly
Test1::Test1 (Foo const& foo) : pimpl_type(foo) {}                  // Make sure 'const' handled properly
Test1::Test1 (Foo*       foo) : pimpl_type(foo) {}                  // Make sure 'const' handled properly
Test1::Test1 (Foo const* foo) : pimpl_type(foo) {}                  // Make sure 'const' handled properly
Test1::Test1 (Foo      & f1, Foo      & f2) : pimpl_type(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo      & f1, Foo const& f2) : pimpl_type(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo const& f1, Foo      & f2) : pimpl_type(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo const& f1, Foo const& f2) : pimpl_type(f1, f2) {} // Make sure 'const' handled properly

static Test1 single;

Test1::Test1 (singleton_type const&) : pimpl_type(single) {} // 'single' is used as a Singleton.

bool
Test2::operator==(Test2 const& that) const
{
    (*this)->trace_ = "Test2::operator==(Test2 const&)";
    return (*this)->int_ == that->int_;
}

///////////////////////////////////////////////////
// Testing polymorphism
///////////////////////////////////////////////////

template<> struct pimpl<Base>::implementation
{
    implementation (int k) : base_int_(k), trace_("Base::implementation(int)") {}
    virtual ~implementation ()            { /*printf("Base::~implementation()\n"); */ }

    virtual std::string call_virtual() { return("Base::call_virtual()"); }

    int      base_int_;
    std::string trace_;
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
    virtual std::string call_virtual() { return ("Derived1::call_virtual()"); }

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
    virtual std::string call_virtual() { return ("Derived2::call_virtual()"); }

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

std::string const&
Base::trace() const
{
    return (*this)->trace_;
}

std::string
Base::call_virtual()
{
    return (*this)->call_virtual();
}
