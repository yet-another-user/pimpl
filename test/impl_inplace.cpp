#include "./test.hpp"

template<> struct boost::impl_ptr<InPlace>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "InPlace()"; }
    implementation (int k) : int_(k)        { trace_ = "InPlace(int)"; }
    implementation (int k, int) : int_(k)   { trace_ = "InPlace(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "InPlace(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "InPlace(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("InPlace(InPlace const&)")
    {}
    this_type& operator=(this_type const& other)
    {
        int_   = other.int_;
        trace_ = "InPlace::operator=(InPlace const&)";

        return *this;
    }
    int              int_;
    mutable string trace_;
};

InPlace::InPlace ()      : impl_ptr_type(in_place) {}
InPlace::InPlace (int k) : impl_ptr_type(in_place, k) {}

string InPlace::trace () const { return *this ? (*this)->trace_ : "null"; }
int    InPlace::value () const { return (*this)->int_; }

bool
InPlace::operator==(InPlace const& that) const
{
    (*this)->trace_ = "InPlace::operator==(InPlace const&)";

    return (*this)->int_ == that->int_;
}

