#include "./test.hpp"

template<> struct boost::impl_ptr<OnStack>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "OnStack()"; }
    implementation (int k) : int_(k)        { trace_ = "OnStack(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "OnStack(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "OnStack(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "OnStack(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("OnStack(OnStack const&)")
    {}
    this_type& operator=(this_type const& other)
    {
        int_   = other.int_;
        trace_ = "OnStack::operator=(OnStack const&)";

        return *this;
    }
    int              int_;
    mutable string trace_;
};

OnStack::OnStack ()      : impl_ptr_type(in_place) {}
OnStack::OnStack (int k) : impl_ptr_type(in_place, k) {}

string OnStack::trace () const { return *this ? (*this)->trace_ : "null"; }
int    OnStack::value () const { return (*this)->int_; }

bool
OnStack::operator==(OnStack const& that) const
{
    (*this)->trace_ = "OnStack::operator==(OnStack const&)";

    return (*this)->int_ == that->int_;
}

