#include "./test.hpp"

template<> struct boost::impl_ptr<AlwaysInPlace>::implementation
{
    using this_type = implementation;

    implementation ()           : int_(0), trace_("AlwaysInPlace()"          ) {}
    implementation (int k)      : int_(k), trace_("AlwaysInPlace(int)"       ) {}
    implementation (int k, int) : int_(k), trace_("AlwaysInPlace(int, int)"  ) {}
    implementation (Foo&)       : int_(0), trace_("AlwaysInPlace(Foo&)"      ) {}
    implementation (Foo const&) : int_(0), trace_("AlwaysInPlace(Foo const&)") {}

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("AlwaysInPlace(AlwaysInPlace const&)")
    {}
    this_type& operator=(this_type const& other)
    {
        int_   = other.int_;
        trace_ = "AlwaysInPlace::operator=(AlwaysInPlace const&)";

        return *this;
    }
    int                 int_;
    mutable const char* trace_;
};

static_assert(sizeof(AlwaysInPlace) == sizeof(boost::impl_ptr<AlwaysInPlace>::implementation),
        "No memory size overhead for always_inplace is permitted");
static_assert(alignof(AlwaysInPlace) == alignof(boost::impl_ptr<AlwaysInPlace>::implementation),
        "No memory alignment overhead for always_inplace is permitted");

AlwaysInPlace::AlwaysInPlace ()      : impl_ptr_type(in_place) {}
AlwaysInPlace::AlwaysInPlace (int k) : impl_ptr_type(in_place, k) {}

string AlwaysInPlace::trace () const { return *this ? (*this)->trace_ : "null"; }
int    AlwaysInPlace::value () const { return (*this)->int_; }

bool
AlwaysInPlace::operator==(AlwaysInPlace const& that) const
{
    (*this)->trace_ = "AlwaysInPlace::operator==(AlwaysInPlace const&)";

    return (*this)->int_ == that->int_;
}
