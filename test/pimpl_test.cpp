#include <boost/detail/lightweight_test.hpp>
#include "./pimpl_test.hpp"

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
  
Book::Book(std::string const& title, std::string const& author) : pimpl_base(title, author)
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
    BOOST_TEST(false == is_pimpl<Foo>::value);
    BOOST_TEST(false == is_pimpl<int>::value);
    BOOST_TEST(false == is_pimpl<int*>::value);
    BOOST_TEST(false == is_pimpl<int const&>::value);
    BOOST_TEST(true  == is_pimpl<Test1>::value);
    BOOST_TEST(false == is_pimpl<Test1*>::value);
    BOOST_TEST(true  == is_pimpl<Test2>::value);
    BOOST_TEST(true  == is_pimpl<Base>::value);
    BOOST_TEST(true  == is_pimpl<Derived1>::value);
    BOOST_TEST(true  == is_pimpl<Derived1 const>::value);
    BOOST_TEST(true  == is_pimpl<Derived1 const&>::value);
}

static
void
test_swap()
{
    singleton_type single;

    Test1 pt16 (single);
    Test1 pt17 (single);              BOOST_TEST(pt16.id() == pt17.id()); // No copying. Implementation shared.
    Test2 vt12 (5);                      BOOST_TEST(vt12.trace() == "Test2::implementation(int)");
    Test2 vt13 = vt12;
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
test_serialization()
{
#ifdef DISABLED_NOR_NOW
    printf("BEG: Testing pimpl and boost::serialization.\n");
    Test1                      saved (12345);
    std::ofstream                ofs ("filename");
    boost::archive::text_oarchive oa (ofs);

    printf("Calling boost::archive::test_oarchive << Test1.\n");
    oa << *(Test1 const*) &saved; // write class instance to archive
    ofs.close();

    std::ifstream                ifs ("filename", std::ios::binary);
    boost::archive::text_iarchive ia (ifs);
    Test1                   restored (Test1::null()); // Implementation will be replaced.

    printf("Calling boost::archive::test_iarchive >> Test1.\n");
    ia >> restored; // read class state from archive
    assert(saved.get() == restored.get());
    printf("END: Testing pimpl and boost::serialization.\n");
#endif
}

static
void
test_runtime_polymorphic_behavior()
{
    Base        base1 (1);
    Derived1 derived1 (2, 3);
    Derived2 derived2 (2, 3, 4);
    Base        base2 (derived1); // calls copy constructor
    Base        base3 (derived2); // calls copy constructor
    Base        base4 (Derived2(2,3,4)/*const ref to temporary*/); // calls copy copnstructor
    Derived1     bad1 (pimpl<Derived1>::null());
    Derived2     bad2 (pimpl<Derived2>::null());
    Base*         bp1 = &base1;
    Base*         bp2 = &base2;
    Base*         bp3 = &base3;
    Base*         bp4 = &derived1;
    Base*         bp5 = &derived2;
//    Base*         bp6 = &bad1;
//    Base*         bp7 = &bad2;
    Base        bad_base1 = Base::null();
    Base        bad_base2 = pimpl<Base>::null();
//  Base        bad_base3 = Base::null<Foo>(); // correctly does not compile.
//  Foo           bad_foo = null<Foo>(); // correctly does not compile.
    Derived1 bad_derived1 = pimpl<Derived1>::null();
    Derived2 bad_derived2 = pimpl<Derived2>::null();

    if (base1 == derived1); // Make sure base-derived comparisons compile and work
    if (base1 == derived2); // Make sure base-derived comparisons compile and work
    if (derived1 == base1); // Make sure base-derived comparisons compile and work
    if (derived2 == base1); // Make sure base-derived comparisons compile and work

    bool bad1_bool1 =   bad1; // Test conversion to bool
    bool bad1_bool2 = !!bad1; // Test operator!()
    bool bad2_bool1 =   bad2; // Test conversion to bool
    bool bad2_bool2 = !!bad2; // Test operator!()

    BOOST_TEST(!bad1_bool1);
    BOOST_TEST(!bad1_bool2);
    BOOST_TEST(!bad2_bool1);
    BOOST_TEST(!bad2_bool2);

    base2.call_virtual(); // calls Derived1::call_virtual
    base3.call_virtual(); // calls Derived2::call_virtual

    bp1->call_virtual();    // calls Base::call_virtual
    bp2->call_virtual();    // calls Derived1::call_virtual
    bp3->call_virtual();    // calls Derived2::call_virtual
    bp4->call_virtual();    // calls Derived1::call_virtual
    bp5->call_virtual();    // calls Derived2::call_virtual
//  bp6->call_virtual();    // crash. referencing bad1
//  bp7->call_virtual();    // crash. referencing bad1
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
    Test2 v13 = v12;                    BOOST_TEST(v13.trace() == "Test2::implementation(implementation const&)");
    Test2 v14(v12);                     BOOST_TEST(v14.trace() == "Test2::implementation(implementation const&)");
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
    bool p1_bool1 =   p1; // Test conversion to bool
    bool p1_bool2 = !!p1; // Test operator!()
    bool v1_bool1 =   v1; // Test conversion to bool
    bool v1_bool2 = !!v1; // Test operator!()
    bool p2_bool1 =   p2; // Test conversion to bool
    bool p2_bool2 = !!p2; // Test operator!()
    bool v2_bool1 =   v2; // Test conversion to bool
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
    Test1 pt11;
    Test2 vt11;
    Test1 pt12(5);
    Test2 vt12(5);
    Test1 pt13 = pt12;
    Test2 vt13 = vt12;

    BOOST_TEST(pt12 == pt13);    // calls pimpl::op==()
    BOOST_TEST(vt12 == vt13);    // calls Test2::op==()
    if (pt12 != pt13); // Step through to make sure it calls pimpl::op!=()
    if (vt12 != vt13); // Step through to make sure it calls pimpl::op!=() and then Test2::op==()

    BOOST_TEST(vt12 == vt13);
    BOOST_TEST(vt12.trace() == "Test2::operator==(Test2 const&)");
    BOOST_TEST(vt11 != vt12);
    BOOST_TEST(vt11.trace() == "Test2::operator==(Test2 const&)");
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
    test_serialization();

    return boost::report_errors();
}

