#ifndef AUXILIARY_PIMPLE_TEST_HPP
#define AUXILIARY_PIMPLE_TEST_HPP

#include "../include/pimpl.hpp"
#include <boost/detail/lightweight_test.hpp>
#include <string>

struct singleton_type {};
struct pass_value_type {};
struct Foo {};

using string = std::string;

struct Test : public pimpl<Test>::pointer_semantics
{
    // Pure interface. The implementation is hidden in pimpl_test_implementation.cpp

    Test ();
    Test (int);
    Test (int, int);
    Test (Foo&);
    Test (Foo const&);
    Test (Foo const&, Foo const&);
    Test (Foo&, Foo const&);
    Test (Foo const&, Foo&);
    Test (Foo&, Foo&);
    Test (Foo*);
    Test (Foo const*);
    Test (singleton_type const&);
    Test (pass_value_type const&);

    string const& trace () const;
    int              id () const;
};

struct Value : public pimpl<Value>::value_semantics
{
    // Pure interface. The implementation is hidden in pimpl_test_implementation.cpp

    Value ();
    Value (int);

    // Value-semantics Pimpl must explicitly define comparison operators
    // if it wants to be comparable. The same as normal classes do.
    bool operator==(Value const& o) const;
    bool operator!=(Value const& o) const { return !operator==(o); }

    string const& trace () const;
    int              id () const;
};

struct Base : public pimpl<Base>::pointer_semantics
{
    Base (int);

    string call_virtual(); // Non-virtual.
    string const& trace() const;
};

struct Derived1 : public Base
{
    Derived1 (int, int);
};

struct Derived2 : public Derived1
{
    Derived2 (int, int, int);
};


#endif // AUXILIARY_PIMPLE_TEST_HPP
