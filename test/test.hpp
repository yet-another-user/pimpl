#ifndef AUXILIARY_PIMPLE_TEST_HPP
#define AUXILIARY_PIMPLE_TEST_HPP

#include "../include/impl_ptr.hpp"
#include <boost/detail/lightweight_test.hpp>
#include <string>

using string = std::string;

struct Book : boost::impl_ptr<Book>::shared
{
    Book(string const& title, string const& author);

    string const&  title () const;
    string const& author () const;
};

struct singleton_type {};
struct Foo {};

struct Shared : boost::impl_ptr<Shared>::shared // Pure interface.
{
    Shared ();
    Shared (int);
    Shared (int, int);
    Shared (Foo&);
    Shared (Foo const&);
    Shared (Foo const&, Foo const&);
    Shared (Foo&, Foo const&);
    Shared (Foo const&, Foo&);
    Shared (Foo&, Foo&);
    Shared (Foo*);
    Shared (Foo const*);
    Shared (singleton_type);

    string trace () const;
    int    value () const;
    int       id () const;
};

struct Value : boost::impl_ptr<Value>::unique // Pure interface.
{
    Value ();
    Value (int);

    // Value-semantics Pimpl must explicitly define comparison operators
    // if it wants to be comparable. The same as normal classes do.
    bool operator==(Value const& o) const;
    bool operator!=(Value const& o) const { return !operator==(o); }

    string trace () const;
    int    value () const;
    int       id () const;
};

struct Base : boost::impl_ptr<Base>::shared
{
    Base (int);

    string call_virtual(); // Non-virtual.
    string trace() const;
};

struct Derived1 : Base { Derived1 (int, int); };
struct Derived2 : Derived1 { Derived2 (int, int, int); };

#endif // AUXILIARY_PIMPLE_TEST_HPP
