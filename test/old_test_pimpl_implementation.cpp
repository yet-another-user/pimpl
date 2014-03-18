#include "test_pimpl.h"

#if defined(__linux__) && 1 < 0
#	include <boost/archive/binary_iarchive.hpp>
#	include <boost/archive/binary_oarchive.hpp>
#	include <boost/archive/text_iarchive.hpp>
#	include <boost/archive/text_oarchive.hpp>
#	include <boost/archive/xml_iarchive.hpp>
#	include <boost/archive/xml_oarchive.hpp>

BOOST_SERIALIZATION_PIMPL_REPLACE(Test1);

#endif

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
   ~implementation ()                       { trace_ =  "Test1::~implementation()"; }

    int int_;
    mutable string trace_;
    uid id_;

    template<class Archive>
    void
    serialize(Archive& a, unsigned int file_version)
    {
        trace_ =  "pimpl<Test1>::implementation::serialize()";
        a & BOOST_SERIALIZATION_NVP(int_);
    }
};

template<> struct pimpl<Test2>::implementation
{
    implementation () : int_(0)             { trace_ =  "Test2::implementation()"; }
    implementation (int k) : int_(k)        { trace_ =  "Test2::implementation(int)"; }
    implementation (int k, int l) : int_(k) { trace_ =  "Test2::implementation(int, int)"; }
    implementation (Foo&)                   { trace_ =  "Test2::implementation(Foo&)"; }
    implementation (Foo const&)             { trace_ =  "Test2::implementation(Foo const&)"; }
   ~implementation ()                       { trace_ =  "Test2::~implementation()"; }

    implementation(implementation const& that)
    :
        int_(that.int_), trace_("Test2::implementation(implementation const&)")
    {}

    int int_;
    mutable string trace_;
    uid id_;
};

typedef Foo const& cref;

//Test1::Test1(pass_value_type) : base(Foo::create()) {}
Test1::Test1(pass_value_type) : base(cref(create_foo())) {}

int           Test1::  get () const { return (*this)->int_; }
int           Test2::  get () const { return (*this)->int_; }
string const& Test1::trace () const { return (*this)->trace_; }
string const& Test2::trace () const { return (*this)->trace_; }
int           Test1::   id () const { return (*this)->id_; }
int           Test2::   id () const { return (*this)->id_; }

Test1::Test1 () : base() {}                     // Call implementation::implementation()
Test2::Test2 () : base() {}                     // ditto
Test1::Test1 (int k) : base(k) {}               // Call implementation::implementation(int)
Test2::Test2 (int k) : base(k) {}               // ditto
Test1::Test1 (int k, int l) : base(k, l) {}     // Call implementation::implementation(int, int)

Test1::Test1 (Foo&       foo) : base(foo) {}    // Make sure 'const' handled properly
Test1::Test1 (Foo const& foo) : base(foo) {}    // Make sure 'const' handled properly
Test1::Test1 (Foo*       foo) : base(foo) {}    // Make sure 'const' handled properly
Test1::Test1 (Foo const* foo) : base(foo) {}    // Make sure 'const' handled properly
Test1::Test1 (Foo      & f1, Foo      & f2) : base(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo      & f1, Foo const& f2) : base(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo const& f1, Foo      & f2) : base(f1, f2) {} // Make sure 'const' handled properly
Test1::Test1 (Foo const& f1, Foo const& f2) : base(f1, f2) {} // Make sure 'const' handled properly

static Test1 single;
static Test1 const const_single;
Test1::Test1 (singleton_type) : base(single) {} // 'single' is used as a Singleton.

void
Test2::set(int v)
{
    (*this)->int_ = v;
}

bool
Test2::operator==(Test2 const& that) const
{
    (*this)->trace_ = "Test2::operator==(Test2 const&)";
    return (*this)->int_ == that->int_;
}

///////////////////////////////////////////////////
// Testing polymorphism
///////////////////////////////////////////////////

typedef pimpl<Base>::implementation BaseImpl;

template<> struct pimpl<Base>::implementation
{
    implementation (int k) : base_int_(k) { printf("Base::implementation(int)\n"); }
    virtual ~implementation ()            { printf("Base::~implementation()\n"); }
    virtual void call_virtual()           { printf("Base::call_virtual()\n"); }

    int base_int_;
};

struct Derived1Impl : public BaseImpl
{
    typedef BaseImpl base;

    Derived1Impl (int k, int l) : base(k), derived_int_(l)
    {
        printf("Derived1::implementation(int,int)\n");
    }
   ~Derived1Impl ()
    {
        printf("Derived1::~implementation()\n");
    }
    virtual void call_virtual() { printf("Derived1::call_virtual()\n"); }

    int derived_int_;
};

struct Derived2Impl : public Derived1Impl
{
    typedef Derived1Impl base;

    Derived2Impl (int k, int l, int m) : base(k, l), more_int_(m)
    {
        printf("Derived2::implementation(int, int)\n");
    }
   ~Derived2Impl ()
    {
        printf("Derived2::~implementation()\n");
    }
    virtual void call_virtual() { printf("Derived2::call_virtual()\n"); }

    int more_int_;
};

Base::Base(int k) : base(k)
{
}

Derived1::Derived1(int k, int l) : Base(null<Base>())
{
    reset(new Derived1Impl(k, l));
}

Derived2::Derived2(int k, int l, int m) : Derived1(null<Derived1>())
{
    reset(new Derived2Impl(k, l, m));
}

void
Base::call_virtual()
{
    (*this)->call_virtual();
}
