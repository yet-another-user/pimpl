#include "./test.hpp"

template<> struct boost::impl_ptr<Base>::implementation
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

struct Derived1Impl : boost::impl_ptr<Base>::implementation
{
    using this_impl = Derived1Impl;
    using base_impl = boost::impl_ptr<Base>::implementation;

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
    using base_impl = Derived1Impl;
    using this_impl = Derived2Impl;

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

Derived1::Derived1(int k, int l) : Base(null())
{
    emplace<Derived1Impl>(k, l);
}

Derived2::Derived2(int k, int l, int m) : Derived1(null<Derived1>())
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
