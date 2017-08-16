#include "./test.hpp"
#include "./allocator.hpp"

// This internal implementation usually only have destructor, constructors,
// data and probably internal methods. Given it is already internal by design,
// it does not need to be a 'class'. All public methods are declared in the
// external visible Shared class. Then, data in this structure are accessed as
// (*this)->data or (**this).data.
template<> struct boost::impl_ptr<Shared>::implementation
{
    using this_type = implementation;

    implementation ()                       { trace_ = "Shared()"; }
    implementation (int k) : int_(k)        { trace_ = "Shared(int)"; }
    implementation (int k, int l) : int_(k) { trace_ = "Shared(int, int)"; }
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

Shared::Shared ()               : impl_ptr_type(in_place) {}
Shared::Shared (int k)          : impl_ptr_type(in_place, k) {}
Shared::Shared (int k, int l)   : impl_ptr_type(in_place, k, l) {}
Shared::Shared (Foo&       foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const& foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo*       foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly
Shared::Shared (Foo const* foo) : impl_ptr_type(in_place, foo) {} // Make sure 'const' handled properly

Shared::Shared (test::singleton_type) : impl_ptr_type(null())
{
    static Shared single = Shared::null();
    static bool   inited = (single.emplace(), true);

    *this = single;
}

Shared::Shared(test::allocator_type)
:
    impl_ptr_type(in_place, my_allocator<implementation>())
{}

