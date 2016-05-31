#ifndef AUXILIARY_PIMPLE_TEST_HPP
#define AUXILIARY_PIMPLE_TEST_HPP

#include "../include/pimpl.hpp"
#include <boost/detail/lightweight_test.hpp>
#include <string>

using string = std::string;

struct Book : public boost::pimpl<Book>::shared
{
    Book();
    Book(string const& title, string const& author);

    string const&  title () const;
    string const& author () const;
};

struct singleton_type {};
struct Foo {};

struct Shared : public boost::pimpl<Shared>::shared // Pure interface.
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
    int       id () const;
};

struct Value : public boost::pimpl<Value>::unique // Pure interface.
{
    Value ();
    Value (int);

    // Value-semantics Pimpl must explicitly define comparison operators
    // if it wants to be comparable. The same as normal classes do.
    bool operator==(Value const& o) const;
    bool operator!=(Value const& o) const { return !operator==(o); }

    string trace () const;
    int       id () const;
};

struct Base : public boost::pimpl<Base>::shared
{
    Base (int);

    string call_virtual(); // Non-virtual.
    string trace() const;
};

struct Derived1 : public Base { Derived1 (int, int); };
struct Derived2 : public Derived1 { Derived2 (int, int, int); };

#endif // AUXILIARY_PIMPLE_TEST_HPP
