#include "./pimpl_test.hpp"
#include <set>

struct Book : public pimpl<Book>::pointer_semantics
{
    Book(std::string const& title, std::string const& author);

    std::string const&  title () const;
    std::string const& author () const;
};

template<>
struct pimpl<Book>::implementation
{
    implementation(std::string const& the_title, std::string const& the_author)
    :
        title(the_title), author(the_author)
    {
    }

    std::string  title;
    std::string author;
};

Book::Book(std::string const& title, std::string const& author) : base(title, author)
{
}

std::string const& Book:: title() const { return (*this)->title; }
std::string const& Book::author() const { return (*this)->author; }

static
void
test_basics()
{
    Book book("Dune", "Frank Herbert");

    BOOST_TEST(book.title() == "Dune");
    BOOST_TEST(book.author() == "Frank Herbert");
}

static
void
test_is_pimpl()
{
    BOOST_TEST(false == pimpl<Foo>::value);
    BOOST_TEST(false == pimpl<int>::value);
    BOOST_TEST(false == pimpl<int*>::value);
    BOOST_TEST(false == pimpl<int const&>::value);
    BOOST_TEST(true  == pimpl<Test1>::value);
    BOOST_TEST(false == pimpl<Test1*>::value);
    BOOST_TEST(true  == pimpl<Test2>::value);
    BOOST_TEST(true  == pimpl<Base>::value);
    BOOST_TEST(true  == pimpl<Derived1>::value);
    BOOST_TEST(true  == pimpl<Derived1 const>::value);
    BOOST_TEST(true  == pimpl<Derived1 const&>::value);
}

static
void
test_swap()
{
    singleton_type single;

    Test1 pt16 (single);
    Test1 pt17 (single); BOOST_TEST(pt16.id() == pt17.id()); // No copying. Implementation shared.
    Test2 vt12 (5);      BOOST_TEST(vt12.trace() == "Test2::implementation(int)");
    Test2 vt13 = vt12;   BOOST_TEST(vt13.trace() == "Test2::implementation(Test2::implementation const&)");
    Test2 vt14 (vt12);

    int pt16_id = pt16.id();
    int pt17_id = pt17.id();
    int vt13_id = vt13.id();
    int vt14_id = vt14.id();

    pt16.swap(pt17);
    vt13.swap(vt14);

    BOOST_TEST(pt16.id() == pt17_id);
    BOOST_TEST(pt17.id() == pt16_id);
    BOOST_TEST(vt13.id() == vt14_id);
    BOOST_TEST(vt14.id() == vt13_id);
}

static
void
test_runtime_polymorphic_behavior()
{
    Base        base1 (1);
    Derived1 derived1 (1, 2);
    Derived2 derived2 (1, 2, 3);
    Base        base2 (derived1);
    Base        base3 (derived2);
    Base        base4 (Derived2(2,3,4)/*const ref to temporary*/);
    Base*         bp1 = &base1;
    Base*         bp2 = &base2;
    Base*         bp3 = &base3;
    Base*         bp4 = &derived1;
    Base*         bp5 = &derived2;
    Base         bad1 = Base::null();
    Base         bad2 = pimpl<Base>::null();
    Base         bad3 = pimpl<Derived1>::null();
    Base         bad4 = pimpl<Derived2>::null();
    Derived1     bad5 (pimpl<Derived1>::null());
    Derived2     bad6 (pimpl<Derived2>::null());

    BOOST_TEST(derived1.trace() == "Derived1::implementation(int, int)");
    BOOST_TEST(derived2.trace() == "Derived2::implementation(int, int, int)");

    BOOST_TEST(base1 != derived1);
    BOOST_TEST(base1 != derived2);
    BOOST_TEST(base2 == derived1);
    BOOST_TEST(base3 == derived2);
    BOOST_TEST(derived1 != base1);
    BOOST_TEST(derived2 != base1);
    BOOST_TEST(derived1 == base2);
    BOOST_TEST(derived2 == base3);

    bool bad1_bool1 =   bad5 ? true : false; // Test conversion to bool
    bool bad1_bool2 = !!bad5; // Test operator!()
    bool bad2_bool1 =   bad6 ? true : false; // Test conversion to bool
    bool bad2_bool2 = !!bad6; // Test operator!()

    BOOST_TEST(!bad1_bool1);
    BOOST_TEST(!bad1_bool2);
    BOOST_TEST(!bad2_bool1);
    BOOST_TEST(!bad2_bool2);

    BOOST_TEST(base2.call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(base3.call_virtual() == "Derived2::call_virtual()");
    BOOST_TEST(bp1->call_virtual() == "Base::call_virtual()");
    BOOST_TEST(bp2->call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(bp3->call_virtual() == "Derived2::call_virtual()");
    BOOST_TEST(bp4->call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(bp5->call_virtual() == "Derived2::call_virtual()");
}

static
void
test_constructors()
{
    singleton_type singleton;
    pass_value_type pass_value;
    Foo              foo;
    Foo const  const_foo = foo;
    Foo&             ref = foo;
    Foo const& const_ref = const_foo;
    Foo*             ptr = &foo;
    Foo const* const_ptr = &const_foo;

    Test1 p11;                          BOOST_TEST(p11.trace() == "Test1::implementation()");
    Test2 v11;                          BOOST_TEST(v11.trace() == "Test2::implementation()");
    Test1 p12(5);                       BOOST_TEST(p12.trace() == "Test1::implementation(int)");
    Test2 v12(5);                       BOOST_TEST(v12.trace() == "Test2::implementation(int)");
    Test1 p13 = p12;                    BOOST_TEST(p13.id() == p12.id()); // No copying. Implementation shared.
    Test1 p14(p12);                     BOOST_TEST(p14.id() == p12.id()); // No copying. Implementation shared.
                                        BOOST_TEST(p13.trace() == "Test1::implementation(int)"); // trace state is the same
                                        BOOST_TEST(p14.trace() == "Test1::implementation(int)"); // trace state is the same
    Test2 v13 = v12;                    BOOST_TEST(v13.trace() == "Test2::implementation(Test2::implementation const&)");
    Test2 v14(v12);                     BOOST_TEST(v14.trace() == "Test2::implementation(Test2::implementation const&)");
                                        BOOST_TEST(v13.id() != v12.id()); // Implementation copied.
                                        BOOST_TEST(v14.id() != v12.id()); // Implementation copied.
    Test1 p15(5, 6);                    BOOST_TEST(p15.trace() == "Test1::implementation(int, int)");
    Test1 p16(singleton);
    Test1 p17(singleton);               BOOST_TEST(p16.id() == p17.id()); // No copying. Implementation shared.

    Test1 p21(foo);                     BOOST_TEST(p21.trace() == "Test1::implementation(Foo&)");
    Test1 p22(const_foo);               BOOST_TEST(p22.trace() == "Test1::implementation(Foo const&)");
    Test1 p23(ref);                     BOOST_TEST(p23.trace() == "Test1::implementation(Foo&)");
    Test1 p24(const_ref);               BOOST_TEST(p24.trace() == "Test1::implementation(Foo const&)");
    Test1 p25(ptr);                     BOOST_TEST(p25.trace() == "Test1::implementation(Foo*)");
    Test1 p26(const_ptr);               BOOST_TEST(p26.trace() == "Test1::implementation(Foo const*)");

    Test1 p31(const_foo, const_foo);    BOOST_TEST(p31.trace() == "Test1::implementation(Foo const&, Foo const&)");
    Test1 p32(foo, const_foo);          BOOST_TEST(p32.trace() == "Test1::implementation(Foo&, Foo const&)");
    Test1 p33(const_foo, foo);          BOOST_TEST(p33.trace() == "Test1::implementation(Foo const&, Foo&)");
    Test1 p34(foo, foo);                BOOST_TEST(p34.trace() == "Test1::implementation(Foo&, Foo&)");
    Test1 p35(foo, Foo());              BOOST_TEST(p35.trace() == "Test1::implementation(Foo&, Foo const&)");
    Test1 p36(Foo(), foo);              BOOST_TEST(p36.trace() == "Test1::implementation(Foo const&, Foo&)");
    Test1 p37(pass_value);              BOOST_TEST(p37.trace() == "Test1::implementation(Foo const&)");
}

static
void
test_bool_conversions()
{
    Test1      p1; BOOST_TEST(p1.trace() == "Test1::implementation()");
    Test2      v1; BOOST_TEST(v1.trace() == "Test2::implementation()");
    Test1      p2 = pimpl<Test1>::null();
    Test2      v2 = pimpl<Test2>::null();
    bool p1_bool1 =   p1 ? true : false; // Test conversion to bool
    bool p1_bool2 = !!p1; // Test operator!()
    bool v1_bool1 =   v1 ? true : false; // Test conversion to bool
    bool v1_bool2 = !!v1; // Test operator!()
    bool p2_bool1 =   p2 ? true : false; // Test conversion to bool
    bool p2_bool2 = !!p2; // Test operator!()
    bool v2_bool1 =   v2 ? true : false; // Test conversion to bool
    bool v2_bool2 = !!v2; // Test operator!()

    BOOST_TEST( p1_bool1);
    BOOST_TEST( p1_bool2);
    BOOST_TEST( v1_bool1);
    BOOST_TEST( v1_bool2);
    BOOST_TEST(!p2_bool1);
    BOOST_TEST(!p2_bool2);
    BOOST_TEST(!v2_bool1);
    BOOST_TEST(!v2_bool2);
}

static
void
test_comparisons()
{
    Test1 p1;
    Test2 v1;
    Test1 p2(5);
    Test2 v2(5);
    Test1 p3 = p2;
    Test2 v3 = v2;

    BOOST_TEST(p2 == p3); // calls pimpl::op==()
    BOOST_TEST(v2 == v3);
    BOOST_TEST(v2.trace() == "Test2::operator==(Test2 const&)");
    BOOST_TEST(v1 != v2);
    BOOST_TEST(v1.trace() == "Test2::operator==(Test2 const&)");
}

static
void
test_singleton()
{
    singleton_type single;

    Test1 p1 (single);
    Test1 p2 (single);
    std::set<Test1> collected;

    collected.insert(p1);
    collected.insert(p2);

    BOOST_TEST(p1 == p2);       // Equality test
    BOOST_ASSERT (!(p1 < p2));  // Equivalence test
    BOOST_ASSERT (!(p2 < p1));  // Equivalence test
    BOOST_TEST(collected.size() == 1);
}

int
main(int argc, char const* argv[])
{
    test_basics();
    test_is_pimpl();
    test_constructors();
    test_bool_conversions();
    test_comparisons();
    test_runtime_polymorphic_behavior();
    test_swap();
    test_singleton();

    return boost::report_errors();
}

