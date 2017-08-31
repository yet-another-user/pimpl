#ifndef IMPL_PTR_TEST_HPP
#define IMPL_PTR_TEST_HPP

#include "../include/impl_ptr.hpp"
#include <boost/detail/lightweight_test.hpp>
#include <string>

using string = std::string;

//struct Book : boost::impl_ptr<Book>::shared
struct Book : boost::impl_ptr<Book>::policy<detail::shared>
{
    Book(string const& title, string const& author);

    string const&  title () const;
    string const& author () const;
};

struct test
{
    struct singleton_type {};

    static constexpr singleton_type singleton {};
};

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
    Shared (test::singleton_type);

    string trace () const;
    int    value () const;
};

struct Unique : boost::impl_ptr<Unique>::unique // Pure interface.
{
    Unique ();
    Unique (int);

    string trace () const;
    int    value () const;
};

struct Copied : boost::impl_ptr<Copied>::copied // Pure interface.
{
    Copied ();
    Copied (int);

    // Value-semantics Pimpl must explicitly define comparison operators
    // if it wants to be comparable. The same as normal classes do.
    bool operator==(Copied const& o) const;
    bool operator!=(Copied const& o) const { return !operator==(o); }

    string trace () const;
    int    value () const;
};

//struct OnStack : boost::impl_ptr<OnStack, int[16]>::onstack
struct OnStack : boost::impl_ptr<OnStack, int[16]>::policy<detail::onstack>
{
    OnStack ();
    OnStack (int);

    // Value-semantics Pimpl must explicitly define comparison operators
    // if it wants to be comparable. The same as normal classes do.
    bool operator==(OnStack const& o) const;
    bool operator!=(OnStack const& o) const { return !operator==(o); }

    string trace () const;
    int    value () const;
};

struct Base : boost::impl_ptr<Base>::shared
{
    Base (int);

    string call_virtual(); // Non-virtual.
    string trace() const;
};

struct Derived1 : Base { Derived1 (int, int); };
struct Derived2 : Derived1 { Derived2 (int, int, int); };

#endif // IMPL_PTR_TEST_HPP
