#include "./test.hpp"
#include "./allocator.hpp"

template<> struct boost::impl_ptr<Shared>::implementation
{
    using this_type = implementation;

    implementation ()                       { trace_ = "Shared()"; }
    implementation (int k) : int_(k)        { trace_ = "Shared(int)"; }
    implementation (int k, int) : int_(k)   { trace_ = "Shared(int, int)"; }
    implementation (Foo&)                   { trace_ = "Shared(Foo&)"; }
    implementation (Foo const&)             { trace_ = "Shared(Foo const&)"; }
    implementation (Foo*)                   { trace_ = "Shared(Foo*)"; }
    implementation (Foo const*)             { trace_ = "Shared(Foo const*)"; }

    implementation(this_type const&) =delete;
    this_type& operator=(this_type const&) =delete;

    void* operator new(size_t sz)
    {
        BOOST_ASSERT(0); // Never called. Use allocators for custom mem. mgmt
        return malloc(sz);
    };
    void operator delete(void* p, size_t)
    {
        BOOST_ASSERT(0); // Never called. Use allocators for custom mem. mgmt
        if (p) free(p);
    }
    int           int_ = 0;
    std::string trace_;
};

string Shared::trace () const { return *this ? (*this)->trace_ : "null"; }
int    Shared::value () const { return (*this)->int_; }

Shared::Shared ()             : impl_ptr_type(in_place) {}
Shared::Shared (int k)        : impl_ptr_type(in_place, k) {}
Shared::Shared (int k, int l) : impl_ptr_type(in_place, k, l) {}
Shared::Shared (Foo&       f) : impl_ptr_type(in_place, f) {} // Testing that 'const' handled properly
Shared::Shared (Foo const& f) : impl_ptr_type(in_place, f) {} // Testing that 'const' handled properly
Shared::Shared (Foo*       f) : impl_ptr_type(in_place, f) {} // Testing that 'const' handled properly
Shared::Shared (Foo const* f) : impl_ptr_type(in_place, f) {} // Testing that 'const' handled properly

Shared::Shared (test::singleton_type) : impl_ptr_type(nullptr)
{
    static Shared single = boost::impl_ptr<Shared>::null();
    static bool   inited = (single.emplace(), true);

    BOOST_ASSERT(inited);

    *this = single;
}
