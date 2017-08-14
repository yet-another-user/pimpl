#include "./test.hpp"

template<> struct boost::impl_ptr<Copied>::implementation
{
    using this_type = implementation;

    implementation () : int_(0)             { trace_ = "Copied()"; }
    implementation (int k) : int_(k)        { trace_ = "Copied(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Copied(int, int)"; }
    implementation (Foo&) : int_(0)         { trace_ = "Copied(Foo&)"; }
    implementation (Foo const&) : int_(0)   { trace_ = "Copied(Foo const&)"; }

    implementation(this_type const& other)
    :
        int_(other.int_), trace_("Copied(Copied const&)")
    {}
    this_type& operator=(this_type const& other)
    {
        int_   = other.int_;
        trace_ = "Copied::operator=(Copied const&)";

        return *this;
    }

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
};

Copied::Copied ()      : impl_ptr_type(in_place) {}
Copied::Copied (int k) : impl_ptr_type(in_place, k) {}

string Copied::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Copied::value () const { return (*this)->int_; }

bool
Copied::operator==(Copied const& that) const
{
    (*this)->trace_ = "Copied::operator==(Copied const&)";

    return (*this)->int_ == that->int_;
}

