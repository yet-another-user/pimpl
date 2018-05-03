#include "./test.hpp"

template<> struct boost::impl_ptr<Unique>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "Unique::implementation()"; }
    implementation (int k) : int_(k)        { trace_ = "Unique::implementation(int)"; }
    implementation (int k, int) : int_(k)   { trace_ = "Unique::implementation(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "Unique::implementation(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "Unique::implementation(Foo const&)"; }

    implementation (implementation const&) =delete;
    implementation& operator=(implementation const&) =delete;

    int              int_;
    mutable string trace_;
};

Unique::Unique ()      : impl_ptr_type(std::allocator_arg, std::allocator<void>()) {}
Unique::Unique (int k) : impl_ptr_type(in_place, k) {}

string Unique::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Unique::value () const { return (*this)->int_; }

